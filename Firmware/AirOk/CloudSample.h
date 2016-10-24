//#define USE_CLOUD

// SSID of your wifi network
#define CLOUD_SSID "{Your SSID}"

// Password for wifi network
#define CLOUD_PASS "{Your WiFi password}"

// Server host to which data will be sent. "184.106.153.149" for thingspeak.com
#define CLOUD_IP "184.106.153.149"
#define CLOUD_PORT 80

// HTTP request for data sending.
// Replace {WriteAPIKey} with actual key from thingspeak.com
#define CLOUD_GET "GET /update?key={WriteAPIKey}&";

#define CLOUD_FIELD_CO2 F("field1=")
#define CLOUD_FIELD_TEMPERATURE F("field2=")
#define CLOUD_FIELD_PRESSURE F("field3=")
#define CLOUD_FIELD_HUMIDITY F("field4=")

// The below are parameters for uploading data to openhab (openhab.org) home automation service
// It is assumed you have Number Items with names MS_CO2, MS_Temperature, MS_Pressure, MS_Humidity
// Create such items of modify the names below

//// IP address of OpenHab server
//#define CLOUD_IP F("192.168.1.11")
//// Port on which OpenHab is working
//#define CLOUD_PORT 8080
//
//// HTTP request for data sending.
//// It uses openhab REST API (https://github.com/openhab/openhab/wiki/REST-API)
//#define CLOUD_GET F("GET /CMD?")
//
//// OpenHAB parameters, change the names if needed
//#define CLOUD_FIELD_CO2 F("MS_CO2=")
//#define CLOUD_FIELD_TEMPERATURE F("MS_Temperature=")
//#define CLOUD_FIELD_PRESSURE F("MS_Pressure=")
//#define CLOUD_FIELD_HUMIDITY F("MS_Humidity=")
// Optional
//#define CLOUD_FIELD_CO2_RAW F("MS_CO2_Raw=")
