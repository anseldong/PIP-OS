#include "settings.h"
#include <QDir>

namespace PipOS {
Settings::Settings(QObject *parent)
    : QObject(parent), defaultSettings(createDefaultSettings()) {
    qInfo() << "Interface settings being loaded from" << m_settings.fileName();
    initializeDefaults();
}

QMap<QString, QVariant> Settings::createDefaultSettings() {
    return QMap<QString, QVariant>{
        { "Interface/color", "#00ff66" },
        { "Interface/scale", 1.0 },
        { "Interface/xOffset", 0 },
        { "Interface/yOffset", 0 },
        { "Interface/skipBoot", false },
        { "Interface/scanlines", true },
        { "Interface/hideMapTab", false },
        { "Radio/directory", QDir::currentPath() },
        { "Inventory/path", QDir::cleanPath( QDir::currentPath() + "/DemoMode.json" ) },
        { "Map/apiKey", "" },
        { "Map/positionSource", "" },
    };
}

void Settings::initializeDefaults() {
  m_settings.beginGroup(""); // Ensure we're at the root
  QStringList existingKeys = getAllKeys(m_settings);

  // Iterate through all default settings
  for (auto it = defaultSettings.constBegin(); it != defaultSettings.constEnd();
       ++it) {
    if (!existingKeys.contains(it.key())) {
      m_settings.setValue(it.key(), it.value());
    }
  }

  initializeDefaultPerks();
  m_settings.sync(); // Ensure all changes are written to disk
}

void Settings::initializeDefaultPerks()
{
    m_settings.beginGroup("Perks");
    const bool hasPerks = !m_settings.childGroups().isEmpty();
    m_settings.endGroup();

    if (hasPerks)
        return;

    m_settings.setValue("Perks/01/name", "Barbarian");
    m_settings.setValue("Perks/01/stars", 2);
    m_settings.setValue("Perks/01/maxStars", 10);
    m_settings.setValue("Perks/01/description",
                        "Critical Hits with unarmed and melee attacks do +5% damage.");
  m_settings.setValue("Perks/01/image", "/images/perks/grognak.webp");

    m_settings.setValue("Perks/02/name", "Medic");
    m_settings.setValue("Perks/02/stars", 1);
    m_settings.setValue("Perks/02/maxStars", 1);
    m_settings.setValue("Perks/02/description",
                        "Stimpaks restore 40% of lost Health, and RadAway removes 40% of radiation.");
  m_settings.setValue("Perks/02/image", "/images/perks/grognak.webp");

    m_settings.setValue("Perks/03/name", "V.A.N.S.");
    m_settings.setValue("Perks/03/stars", 1);
    m_settings.setValue("Perks/03/maxStars", 1);
    m_settings.setValue("Perks/03/description",
                        "Let Vault-Tec guide you! The path to your closest quest target is displayed in V.A.T.S.");
  m_settings.setValue("Perks/03/image", "/images/perks/grognak.webp");

    m_settings.setValue("Perks/04/name", "Wasteland Survival 8");
    m_settings.setValue("Perks/04/stars", 1);
    m_settings.setValue("Perks/04/maxStars", 1);
    m_settings.setValue("Perks/04/description", "Take 5% less damage from melee attacks.");
  m_settings.setValue("Perks/04/image", "/images/perks/grognak.webp");

    m_settings.setValue("Perks/05/name", "Well Rested");
    m_settings.setValue("Perks/05/stars", 1);
    m_settings.setValue("Perks/05/maxStars", 1);
    m_settings.setValue("Perks/05/description",
                        "You had a great sleep, and feel alert and invigorated! +10% XP earned for a limited time.");
  m_settings.setValue("Perks/05/image", "/images/perks/grognak.webp");
}

QStringList Settings::getAllKeys(QSettings &settings)
{
    QStringList allKeys;
    QStringList groups = settings.childGroups();

    // Get keys in current group
    allKeys.append(settings.childKeys());

    // Recursively get keys from all groups
    for (const QString &group : groups) {
        settings.beginGroup(group);
        QStringList groupKeys = getAllKeys(settings);

        // Prepend group name to keys
        for (const QString &key : groupKeys) {
            allKeys.append(group + "/" + key);
        }
        settings.endGroup();
    }

    return allKeys;
}

QString Settings::interfaceColor() const {
  return m_settings.value("Interface/color").toString();
}

void Settings::setInterfaceColor(const QString &newInterfaceColor) {
  m_settings.setValue("Interface/color", newInterfaceColor);
  emit interfaceColorChanged();
}

bool Settings::skipBoot() const {
  return m_settings.value("Interface/skipBoot").toBool();
}

void Settings::setSkipBoot(bool newSkipBoot) {
  m_settings.setValue("Interface/skipBoot", newSkipBoot);
  emit skipBootChanged();
}

bool Settings::scanLines() const {
    return m_settings.value("Interface/scanlines").toBool();
}

void Settings::setScanLines(bool newScanLines) {
  m_settings.setValue("Interface/scanlines", newScanLines);
  emit scanLinesChanged();
}

QString Settings::radioStationLocation() const {
  return m_settings.value("Radio/directory").toString();
}

QString Settings::inventoryFileLocation() const {
    return m_settings.value( "Inventory/path" ).toString();
}

float Settings::scale() const {
  return m_settings.value("Interface/scale").toFloat();
}
void Settings::setScale(float newScale) {
  m_settings.setValue("Interface/scale", newScale);
  emit scaleChanged();
}
int Settings::xOffset() const {
  return m_settings.value("Interface/xOffset").toInt();
}
void Settings::setXOffset(int newOffset) {
  m_settings.setValue("Interface/xOffset", newOffset);
  emit xOffsetChanged();
}
int Settings::yOffset() const {
  return m_settings.value("Interface/yOffset").toInt();
}
void Settings::setYOffset(int newOffset) {
  m_settings.setValue("Interface/yOffset", newOffset);
  emit yOffsetChanged();
}

QString Settings::mapApiKey() const {
  return m_settings.value("Map/apiKey").toString();
}

QString Settings::mapPositionSource() const
{
    return m_settings.value("Map/positionSource").toString();
}

bool Settings::hideMapTab() const
{
    return m_settings.value("Interface/hideMapTab").toBool();
}

void Settings::setHideMapTab(bool newHideMapTab)
{
    m_settings.setValue("Interface/hideMapTab", newHideMapTab);
    emit hideMapTabChanged();
}

QKeySequence Settings::getKeySequence(Events::UserEvent userEvent) const
{
    return m_keymap[userEvent];
}
} // namespace PipOS
