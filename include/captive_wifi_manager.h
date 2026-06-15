#ifndef CAPTIVEWIFIMANAGER_H
#define CAPTIVEWIFIMANAGER_H

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

#define VALIDATION_BYTE 0x46

namespace captive_wifi
{
    typedef void (*connection_attempt_handler)(const char *ssid);
    typedef void (*ap_created_handler)(const char *ssid, const char *ip);
    typedef void (*connection_result_handler)(bool connected);
    struct wifi_credentials
    {
        char validation_byte;
        char ssid[33];
        char password[64];
    };

    class manager
    {
    public:
        // apSSID – имя создаваемой точки доступа
        // portalHTML – HTML-страница портала (const char*)
        manager(const char *apSSID, const char *portalHTML);

        // Запускает точку доступа, DNS-сервер и веб-сервер
        void begin();

        // Должен вызываться в loop() для обработки запросов
        void handle();

        // Подключение к заданной Wi‑Fi сети (останавливает AP и сервер при успехе)
        void connect(const char *ssid, const char *password);

        // Возвращает true, если устройство подключено к внешней сети
        bool isConnected() const { return _connected; }
        void add_connection_attempt_handler(connection_attempt_handler handler)
        {
            _con_attempt_handler = handler;
        }
        void add_connection_result_handler(connection_result_handler handler)
        {
            _con_res_handler = handler;
        }
        void add_ap_created_handler(ap_created_handler handler)
        {
            _ap_created_handler = handler;
        }

    private:
        void _handle_captive_portal();
        void _handle_submit();
        bool _try_restore_credentials(char *ssid_buf, char *password_buf);
        const char *_ap_ssid;
        const char *_portal_html;
        DNSServer _dns_server;
        WebServer _server;
        bool _connected;
        connection_attempt_handler _con_attempt_handler;
        connection_result_handler _con_res_handler;
        ap_created_handler _ap_created_handler;
        static const unsigned long WIFI_TIMEOUT_MS = 15000;
        static const byte DNS_PORT = 53;
        static const uint32_t EEPROM_ADDRESS = 0;
    };
}

#endif