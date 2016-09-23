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
