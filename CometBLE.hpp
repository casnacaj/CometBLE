/*******************************************************************************
 * @brief Come Blue thermostat Arduino library.
 * @author Jan A. Humpl
 * @license GNU GPL v3 (see LICENSE)
 ******************************************************************************/

#include <BLEDevice.h>
#include <BLEScan.h>

class CometBLE
{
  public:

    static void init();

    static std::string scan(uint32_t timeout_s = 30);

    CometBLE(std::string address, uint32_t password = 0);

    std::string getDeviceName();

    /*
     *  Characteristic value/descriptor: 33 11 0d 03 10
     *                                   |  |  |  |  |
     *                                   |  |  |  |  year
     *                                   |  |  |  month
     *                                   |  |  day of month
     *                                   |  hour
     *                                   minutes
     */
    std::string getDateTime();

    /*
     *  Characteristic value/descriptor: 2d 2a 24 2a 00 04 0a
     *                                   |  |  |  |  |  |  window open minutes
     *                                   |  |  |  |  |  window open detection
     *                                   |  |  |  |  offset temperature
     *                                   |  |  |  target temperature high (0x2a == 21.0 °C)
     *                                   |  |  target temperature low (0x24 == 18.0 °C)
     *                                   |  temperature for manual mode (*2)
     *                                   current temperature (0x2d == 22.5 °C)
     */
    std::string getTemperatures();

    std::string getBattery();
    uint8_t getCurrentTemperature();
    uint8_t getPresetTemperature();

    void setTemperatures(std::string values);
    void setTemperature(uint8_t value);

  private:

    static BLEScan * s_pBLEScan;

    static const size_t DEVICE_ID_LENGTH = 18;
    static const size_t MAX_DEVICE_ID = 8;
    static const size_t DEVICE_ID_BUFFER_LENGTH = MAX_DEVICE_ID * DEVICE_ID_LENGTH;
    static char s_device_id_buffer[DEVICE_ID_BUFFER_LENGTH];
    static std::string s_device_ids;

    BLEClient * const m_pClient;
    const BLEAddress m_address;

    uint32_t m_password;

    std::string getValueNoPasswd(const BLEUUID suuid, const BLEUUID cuuid);
    std::string getValue(const BLEUUID suuid, const BLEUUID cuuid);
    std::string getBinary(const BLEUUID suuid, const BLEUUID cuuid);
    std::string getString(const BLEUUID suuid, const BLEUUID cuuid);
    void setValue(const BLEUUID suuid, const BLEUUID cuuid, const std::string value);

    void open();
    void close();
};
