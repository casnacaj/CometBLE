/*******************************************************************************
 * @brief Come Blue thermostat Arduino library.
 * @author Jan A. Humpl
 * @license GNU GPL v3 (see LICENSE)
 ******************************************************************************/

#include <HardwareSerial.h>
#include "CometBLE.hpp"

extern HardwareSerial Serial;

static const std::string DEVICE_NAME = "Comet Blue";

static const BLEUUID SUUID_DEVICE_NAME = BLEUUID("00001800-0000-1000-8000-00805f9b34fb");
static const BLEUUID CUUID_DEVICE_NAME = BLEUUID("00002a00-0000-1000-8000-00805f9b34fb");
static const BLEUUID SUUID_DATE_TIME = BLEUUID("47e9ee00-47e9-11e4-8939-164230d1df67");
static const BLEUUID CUUID_DATE_TIME = BLEUUID("47e9ee01-47e9-11e4-8939-164230d1df67");
static const BLEUUID SUUID_TEMPERATURES = BLEUUID("47e9ee00-47e9-11e4-8939-164230d1df67");
static const BLEUUID CUUID_TEMPERATURES = BLEUUID("47e9ee2b-47e9-11e4-8939-164230d1df67");
static const BLEUUID SUUID_BATTERY = BLEUUID("47e9ee00-47e9-11e4-8939-164230d1df67");
static const BLEUUID CUUID_BATTERY = BLEUUID("47e9ee2c-47e9-11e4-8939-164230d1df67");
static const BLEUUID SUUID_PASSWORD = BLEUUID("47e9ee00-47e9-11e4-8939-164230d1df67");
static const BLEUUID CUUID_PASSWORD = BLEUUID("47e9ee30-47e9-11e4-8939-164230d1df67");

BLEScan * CometBLE::s_pBLEScan = nullptr;

char CometBLE::s_device_id_buffer[DEVICE_ID_BUFFER_LENGTH] = { 0 };
std::string CometBLE::s_device_ids = std::string(s_device_id_buffer, 0);

void CometBLE::init()
{
  if (s_pBLEScan == nullptr)
  {
    BLEDevice::init("");
    s_pBLEScan = BLEDevice::getScan(); //create new scan
    s_pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  }
}

std::string CometBLE::scan(uint32_t timeout_s)
{
  init();

  BLEScanResults foundDevices = s_pBLEScan->start(timeout_s);
  Serial.print("Devices found: ");
  uint8_t id_found = 0;
  char * ptr_buffer = s_device_id_buffer;
  for (uint8_t i = 0; i < foundDevices.getCount(); i++)
  {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    BLEAddress address = device.getAddress();
    std::string deviceName = BLEDevice::getValue(address, SUUID_DEVICE_NAME, CUUID_DEVICE_NAME);
    if (deviceName.compare(DEVICE_NAME) == 0)
    {
      std::string str_address = address.toString();
      Serial.println(str_address.c_str());
      memcpy(ptr_buffer, str_address.c_str(), str_address.length());
      ptr_buffer += str_address.length();
      *ptr_buffer = ';';
      ptr_buffer++;
      if (++id_found >= MAX_DEVICE_ID)
      {
        break;
      }
    }
  }
  Serial.println("Scan done!");

  s_device_ids = std::string(s_device_id_buffer, static_cast<uintptr_t>(ptr_buffer - s_device_id_buffer));

  return s_device_ids;
}

CometBLE::CometBLE(std::string address, uint32_t password)
  :
    m_pClient(BLEDevice::createClient()),
    m_address(address),
    m_password(password)
{
  init();
}

std::string CometBLE::getDeviceName()
{
  return getString(SUUID_DEVICE_NAME, CUUID_DEVICE_NAME);
}

std::string CometBLE::getDateTime()
{
  return getBinary(SUUID_DATE_TIME, CUUID_DATE_TIME);
}

std::string CometBLE::getTemperatures()
{
  return getBinary(SUUID_TEMPERATURES, CUUID_TEMPERATURES);
}

std::string CometBLE::getBattery()
{
  return getBinary(SUUID_BATTERY, CUUID_BATTERY);
}

uint8_t CometBLE::getCurrentTemperature()
{
  std::string values = getTemperatures();

  uint8_t value = 0;
  if (values.length() == 7)
  {
    value = values[0];
  }

  return value;
}

uint8_t CometBLE::getPresetTemperature()
{
  std::string values = getTemperatures();

  uint8_t value = 0;
  if (values.length() == 7)
  {
    value = values[1];
  }

  return value;
}

std::string CometBLE::getValueNoPasswd(const BLEUUID suuid, const BLEUUID cuuid)
{
  std::string retval;

  try
  {
    BLERemoteService * pValueService = m_pClient->getService(suuid);
    BLERemoteCharacteristic * pValueChar = pValueService->getCharacteristic(cuuid);
    retval = pValueChar->readValue();
  }
  catch (...)
  {
    Serial.print("Exception at line ");
    Serial.println(__LINE__);
    retval = "";
    // Todo.
  }

  return retval;
}

std::string CometBLE::getValue(const BLEUUID suuid, const BLEUUID cuuid)
{
  std::string retval;

  for (uint8_t i = 0; i < 5; i++)
  {
    open();
    retval = getValueNoPasswd(suuid, cuuid);
    close();

    if (retval.length() > 0)
    {
      break;
    }
  }

  return retval;
}

std::string CometBLE::getBinary(const BLEUUID suuid, const BLEUUID cuuid)
{
  std::string retval = getValue(suuid, cuuid);

  Serial.print(":");
  for (size_t i = 0; i < retval.length(); i++)
  {
    Serial.print(" ");
    Serial.print(static_cast<int32_t>(retval[i]));
  }
  Serial.println("\n");

  return retval;
}

std::string CometBLE::getString(const BLEUUID suuid, const BLEUUID cuuid)
{
  std::string retval = getValue(suuid, cuuid);

  Serial.println(retval.c_str());

  return retval;
}

void CometBLE::open()
{
  try
  {
    m_pClient->connect(m_address);

    BLERemoteService * pPasswdService = m_pClient->getService(SUUID_PASSWORD);
    BLERemoteCharacteristic * pPasswdChar = pPasswdService->getCharacteristic(CUUID_PASSWORD);
    pPasswdChar->writeValue(reinterpret_cast<uint8_t*>(&m_password), sizeof(m_password), true);
    pPasswdChar->writeValue(reinterpret_cast<uint8_t*>(&m_password), sizeof(m_password), true);
  }
  catch (...)
  {
    Serial.print("Exception at line ");
    Serial.println(__LINE__);
    // Todo.
  }
}

void CometBLE::close()
{
  try
  {
    m_pClient->disconnect();
  }
  catch (...)
  {
    Serial.print("Exception at line ");
    Serial.println(__LINE__);
    // Todo.
  }
}

void CometBLE::setValue(const BLEUUID suuid, const BLEUUID cuuid, const std::string value)
{
  open();

  try
  {
    BLERemoteService * pValueService = m_pClient->getService(suuid);
    BLERemoteCharacteristic * pValueChar = pValueService->getCharacteristic(cuuid);
    pValueChar->writeValue(value, true);
  }
  catch (...)
  {
    Serial.print("Exception at line ");
    Serial.println(__LINE__);
    // Todo.
  }

  close();
}

void CometBLE::setTemperatures(std::string values)
{
  if (values.length() == 7)
  {
    setValue(SUUID_TEMPERATURES, CUUID_TEMPERATURES, values);
  }
}

void CometBLE::setTemperature(uint8_t value)
{
  std::string values = { 0x80, value, 0x80, 0x80, 0x80, 0x80, 0x80 };

  setTemperatures(values);
}
