// To find the values for these various constants, follow the directions at
// https://docs.arduino.cc/tutorials/opta/getting-started-with-aws-iot-core

// Fill in the name of the device in the IoT console, and the name of the device shadow
#define IOT_DEVICE_NAME ""
#define IOT_DEVICE_SHADOW_NAME ""

// Fill in the hostname of your AWS IoT broker
#define SECRET_BROKER ""

// Fill in the boards public certificate
const char SECRET_CERTIFICATE[] = R"(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)";

class NetworkProfile {
private:
  String ssid;
  String pass;
public:
  NetworkProfile(const String ssid, const String pass):ssid(ssid),pass(pass) {}
  String getSsid() { return this->ssid; }
  String getPass() { return this->pass; }
};

// Define one or more network profiles that you want to allow the device to try to connect to
#define NETWORK_PROFILE_COUNT 1
const NetworkProfile networkProfiles[] = {
  NetworkProfile("ProfileSSIDwithCorrectCase", "PasswordForNetwork"),
};
