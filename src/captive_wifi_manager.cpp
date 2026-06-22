#include <EEPROM.h>
#include "captive_wifi_manager.h"

using namespace captive_wifi;

manager::manager(const char *apSSID, const char *portalHTML)
    : _ap_ssid(apSSID), _portal_html(portalHTML), _server(80), _connected(false), _con_attempt_handler(nullptr), _con_res_handler(nullptr), _ap_created_handler(nullptr)
{
}
void manager::erase_wifi_creds()
{
    wifi_credentials creds = {0x00};
    EEPROM.writeBytes(EEPROM_ADDRESS, &creds, sizeof(wifi_credentials));
    EEPROM.commit();
}
void manager::begin()
{
    static bool restored_con_attempted = false;
    EEPROM.begin(128);
    char ssid_buf[32];
    char password_buf[64];
    if (_try_restore_credentials(ssid_buf, password_buf))
    {
        if (restored_con_attempted)
        {
            erase_wifi_creds();
        }
        else
        {
            restored_con_attempted = true;
            connect(ssid_buf, password_buf);
            return;
        }
    }
    WiFi.mode(WIFI_MODE_AP);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    if (!WiFi.softAP(_ap_ssid))
    {
        log_e("Soft AP creation failed.");
        while (1)
            ;
    }
    if (_ap_created_handler != nullptr)
        _ap_created_handler(_ap_ssid, WiFi.softAPIP().toString().c_str());
    IPAddress myIp = WiFi.softAPIP();
    if (!_dns_server.start(DNS_PORT, "*", myIp))
    {
        log_e("DNS server creation failed.");
        while (1)
            ;
    }
    static char redirect_ip[32] = "/";
    sprintf(redirect_ip, "http://%s/", myIp.toString().c_str());
    // Настройка маршрутов веб-сервера
    _server.onNotFound([this]()
                       {
        this->_server.sendHeader("Location", redirect_ip);
        this->_server.send(302, "text/plain", "redirect to main page"); });
    _server.on("/", [this]()
               { this->_handle_captive_portal(); });
    _server.on("/submit", [this]()
               { this->_handle_submit(); });
    _server.begin();
}

void manager::handle()
{
    if (!_connected)
    {
        _dns_server.processNextRequest();
        _server.handleClient();
    }
}

void manager::connect(const char *ssid, const char *password)
{
    static int32_t reconnect_counter = 0;
    _dns_server.stop();
    _server.stop();

    WiFi.mode(WIFI_MODE_STA);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    log_i("WiFi credentials   SSID: %s, Password: %s", ssid, password);
    WiFi.begin(ssid, (password[0] == '\0') ? nullptr : password);
    if (_con_attempt_handler != nullptr)
        _con_attempt_handler(ssid);
    unsigned long start = millis();
    while (millis() - start < WIFI_TIMEOUT_MS)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            _connected = true;
            log_i("Connected to WiFi");
            if (_con_res_handler != nullptr)
                _con_res_handler(_connected);
            wifi_credentials creds = {VALIDATION_BYTE};
            strcpy(creds.ssid, ssid);
            strcpy(creds.password, password);
            EEPROM.writeBytes(EEPROM_ADDRESS, &creds, sizeof(wifi_credentials));
            EEPROM.commit();
            return;
        }
        delay(50);
    }
    log_e("Failed to connect, reason: %d", WiFi.status());
    _connected = false;
    if (_con_res_handler != nullptr)
        _con_res_handler(_connected);
    begin();
}
void manager::reset()
{
    _connected = false;
    erase_wifi_creds();
    begin();
}
inline void manager::_handle_captive_portal()
{
    _server.send(200, "text/html", _portal_html);
}

void manager::_handle_submit()
{
    String ssid = _server.arg(0);
    String pass = _server.arg(1);
    _server.send(200, "text/plain", "Connecting to WiFi...");
    connect(ssid.c_str(), pass.c_str());
}
bool manager::_try_restore_credentials(char *ssid_buf, char *password_buf)
{
    wifi_credentials creds;
    log_i("Entered restore creds");
    EEPROM.readBytes(EEPROM_ADDRESS, &creds, sizeof(wifi_credentials));
    if (creds.validation_byte != VALIDATION_BYTE)
    {
        log_i("WiFi credentials not found in EEPROM");
        return false;
    }
    log_i("WiFi credentials in EEPROM: ssid - %s, password - %s", creds.ssid, creds.password);
    strcpy(ssid_buf, creds.ssid);
    strcpy(password_buf, creds.password);
    return true;
}