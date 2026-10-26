#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QObject>
#include <QProcessEnvironment>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QUrl>
#include <fstream>
#include <jsoncons/json.hpp>

#include "app.h"

namespace PipOS {

App::App(QObject *parent) : QObject(parent) {
  m_mainWindowEngine = new QQmlApplicationEngine(parent);
  m_settings = new Settings(this);
  m_radio = new Radio(this);
  m_externalData = jsoncons::json::parse(R"({
  })");
}

App::~App() {
  if (m_rotaryProcess) {
    m_rotaryProcess->terminate();
    m_rotaryProcess->waitForFinished(1000);
  }
  if (m_localServer) {
    m_localServer->close();
    QLocalServer::removeServer("/tmp/pipos-control.sock");
  }
}

void App::init() {
  qInfo() << "Init PipOS";

  QDirIterator it(":", {"*.ttf", "*.otf"}, QDir::Files,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    QString font = it.next();
    qDebug() << "Loading font" << font;
    QFontDatabase::addApplicationFont(font);
  }

  // List all contents of QRC
  QDirIterator its(":", QDirIterator::Subdirectories);
  while (its.hasNext()) {
    qDebug() << its.next();
  }

  auto *guiAppInst =
      dynamic_cast<QGuiApplication *>(QGuiApplication::instance());

  // Set a default app wide font
  QFont defaultFont = QFont("Roboto Condensed", 20);
  guiAppInst->setFont(defaultFont);

  try {
    qInfo() << "Loading player inventory from"
            << m_settings->inventoryFileLocation().toStdString();
    std::ifstream is(m_settings->inventoryFileLocation().toStdString());
    m_externalData = jsoncons::json::parse(is);
  } catch (...) {
    qWarning() << "Error loading player inventory";
  }

  m_mainWindowEngine->rootContext()->setContextProperty("app", this);

  // Start IPC server for external input scripts (like rotary encoder)
  startControlServer();

  // Where's all the QML
  m_mainWindowEngine->addImportPath(guiAppInst->applicationDirPath() + "/qml");
  guiAppInst->addLibraryPath(guiAppInst->applicationDirPath() + "/qml");

  // Load the main QML entrypoint
  qInfo() << "Loading main.qml";
  m_mainWindowEngine->load(
      QUrl(QStringLiteral("qrc:/robco-industries.org/PipOS/main.qml")));

  if (m_mainWindowEngine->rootObjects().isEmpty()) {
    qFatal("Failed to load main.qml");
  }

  // Auto-start rotary encoder script process if available
  startRotaryProcess();
}

void App::startControlServer() {
  const QString socketPath = "/tmp/pipos-control.sock";
  QLocalServer::removeServer(socketPath);

  m_localServer = new QLocalServer(this);
  if (m_localServer->listen(socketPath)) {
    qInfo() << "Rotary control socket listening on" << socketPath;
    connect(m_localServer, &QLocalServer::newConnection, this, [this]() {
      while (m_localServer->hasPendingConnections()) {
        QLocalSocket *client = m_localServer->nextPendingConnection();
        connect(client, &QLocalSocket::readyRead, this, [this, client]() {
          while (client->canReadLine()) {
            QString cmd = QString::fromUtf8(client->readLine()).trimmed();
            if (!cmd.isEmpty()) {
              handleCommand(cmd);
            }
          }
        });
        connect(client, &QLocalSocket::disconnected, client,
                &QLocalSocket::deleteLater);
      }
    });
  } else {
    qWarning() << "Failed to start control server on" << socketPath << ":"
               << m_localServer->errorString();
  }
}

void App::startRotaryProcess() {
  QStringList candidates = {
      QDir::currentPath() + "/scripts/rotary_controller.py",
      QDir::currentPath() + "/rotary_controller.py",
      QCoreApplication::applicationDirPath() +
          "/../scripts/rotary_controller.py",
      QCoreApplication::applicationDirPath() + "/scripts/rotary_controller.py",
      "/usr/share/pip-os/scripts/rotary_controller.py"};

  QString scriptPath;
  for (const QString &cand : candidates) {
    if (QFileInfo::exists(cand)) {
      scriptPath = QFileInfo(cand).canonicalFilePath();
      break;
    }
  }

  if (scriptPath.isEmpty()) {
    qInfo() << "No rotary_controller.py found for auto-launch.";
    return;
  }

  m_rotaryProcess = new QProcess(this);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("PYTHONUNBUFFERED", "1");
  m_rotaryProcess->setProcessEnvironment(env);

  connect(m_rotaryProcess, &QProcess::readyReadStandardOutput, this, [this]() {
    while (m_rotaryProcess->canReadLine()) {
      QString line = QString::fromUtf8(m_rotaryProcess->readLine()).trimmed();
      if (!line.isEmpty()) {
        qDebug() << "[Rotary Controller]" << line;
        handleCommand(line);
      }
    }
  });

  connect(m_rotaryProcess, &QProcess::readyReadStandardError, this, [this]() {
    QString err =
        QString::fromUtf8(m_rotaryProcess->readAllStandardError()).trimmed();
    if (!err.isEmpty()) {
      qWarning() << "[Rotary Controller Error]" << err;
    }
  });

  qInfo() << "Starting rotary controller process:" << scriptPath;
  m_rotaryProcess->start("python3", QStringList() << scriptPath);
}

void App::handleCommand(const QString &command) {
  if (command == "SUB_TAB_NEXT") {
    emit subTabNext();
    dispatchUserEvent(Events::UserEvent::SUB_TAB_NEXT);
  } else if (command == "SUB_TAB_PREVIOUS") {
    emit subTabPrevious();
    dispatchUserEvent(Events::UserEvent::SUB_TAB_PREVIOUS);
  } else if (command == "BUTTON_SELECT") {
    emit buttonSelect();
    dispatchUserEvent(Events::UserEvent::BUTTON_SELECT);
  } else if (command == "TAB_STAT") {
    dispatchUserEvent(Events::UserEvent::TAB_STAT);
  } else if (command == "TAB_ITEM") {
    dispatchUserEvent(Events::UserEvent::TAB_ITEM);
  } else if (command == "TAB_DATA") {
    dispatchUserEvent(Events::UserEvent::TAB_DATA);
  } else if (command == "TAB_MAP") {
    dispatchUserEvent(Events::UserEvent::TAB_MAP);
  } else if (command == "TAB_RADIO") {
    dispatchUserEvent(Events::UserEvent::TAB_RADIO);
  } else if (command == "SCROLL_UP") {
    dispatchUserEvent(Events::UserEvent::SCROLL_UP);
  } else if (command == "SCROLL_DOWN") {
    dispatchUserEvent(Events::UserEvent::SCROLL_DOWN);
  }
}

void App::dispatchUserEvent(Events::UserEvent event) {
  emit userEvent(static_cast<int>(event));

  QKeySequence seq = m_settings->getKeySequence(event);
  if (!seq.isEmpty()) {
    int key = seq[0].key();
    QKeyEvent pressEvent(QEvent::KeyPress, key, Qt::NoModifier);
    QKeyEvent releaseEvent(QEvent::KeyRelease, key, Qt::NoModifier);
    for (QObject *obj : m_mainWindowEngine->rootObjects()) {
      QCoreApplication::sendEvent(obj, &pressEvent);
      QCoreApplication::sendEvent(obj, &releaseEvent);
    }
  }
}

} // namespace PipOS
