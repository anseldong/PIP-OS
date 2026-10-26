#pragma once

#include <QGuiApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <jsoncons/json.hpp>

#include "events.h"
#include "radio.h"
#include "settings.h"

namespace PipOS {

class App : public QObject {
  Q_OBJECT
  Q_PROPERTY(Radio *radio READ radio CONSTANT)

public:
  explicit App(QObject *parent = nullptr);
  ~App() override;

  void init();

  // Getters
  QQmlApplicationEngine *mainWindowEngine() const { return m_mainWindowEngine; }
  Radio *radio() const { return m_radio; }
  jsoncons::json externalData() const { return m_externalData; }

signals:
  void userEvent(int event);
  void subTabNext();
  void subTabPrevious();
  void buttonSelect();

public slots:
  void dispatchUserEvent(Events::UserEvent event);
  void handleCommand(const QString &command);

private:
  void startControlServer();
  void startRotaryProcess();

  QQmlApplicationEngine *m_mainWindowEngine = nullptr;
  Settings *m_settings = nullptr;
  Radio *m_radio = nullptr;
  jsoncons::json m_externalData = nullptr;
  QLocalServer *m_localServer = nullptr;
  QProcess *m_rotaryProcess = nullptr;
};
} // namespace PipOS
