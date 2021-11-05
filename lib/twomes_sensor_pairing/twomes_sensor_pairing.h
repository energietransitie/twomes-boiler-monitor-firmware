/**
 * Library for pairing a Twomes sensor with a Twomes Gateway device using ESP-Now
 *
 * Author: Sjors
 * Date: July 2021
 */

#ifndef _TWOMES_SENSOR_PAIRING_H
#define _TWOMES_SENSOR_PAIRING_H

#include <stdlib.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <esp_sleep.h>
#include <sensor_IO.h>

#define ESPNOW_PAIRING_CHANNEL 1
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

const char *TAG = "ESPNOW_PAIRING";

/**
 * @brief read the P1 Gateway Mac and channel from NVS
 *
 * @param macAddress buffer to store the MAC address
 * @param len  length of the buffer (SHOULD ALWAYS BE 6!)
 * @param channel pointer to uint8_t to store channel
 *
 * @return error code
 */
int getGatewayData(uint8_t *macAddress, size_t len, uint8_t *channel) {
  esp_err_t err;
  nvs_handle gatewayHandle;

  //Open the NVS partition:
  err = nvs_open("gatewaySettings", NVS_READONLY, &gatewayHandle);
  if (err != ESP_OK) {
    return err; //return the error
  }

  //read the channel and MAC address:
  err = nvs_get_blob(gatewayHandle, "MAC", macAddress, &len);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Could not read Mac Address!");
    return err;
  }
  err = nvs_get_u8(gatewayHandle, "channel", channel);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Could not read channel!");
    return err;
  }
  return ESP_OK; //Return OK on succes.
}

//Set this as onDataRecv calback when you want to pair. Remove the callback after pairing is done
//ESP automatically reboots when done.
void onDataReceive(const uint8_t *macAddress, const uint8_t *payload, int length) {
  //Read the MAC of the sender and store it in NVS:
  //esp_wifi_stop();
  //esp_wifi_deinit();
  esp_err_t err;
  nvs_handle gatewayHandle;
  ESP_LOGD("Pairing", "Received a pairing message!");
  //Open the NVS partition:
  err = nvs_open("gatewaySettings", NVS_READWRITE, &gatewayHandle);
  if (err == ESP_OK) {
    ESP_ERROR_CHECK(nvs_set_blob(gatewayHandle, "MAC", macAddress, 6));
    ESP_ERROR_CHECK(nvs_set_u8(gatewayHandle, "channel", *payload));
    ESP_LOGD("ESP-Now", "stored MAC address: %02X:%02X:%02X:%02X:%02X:%02X, and Channel %u", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5], *payload);
    gpio_set_level(LED_STATUS, 1);
    gpio_hold_en(LED_STATUS);
    //light sleep while displaying LED
    esp_sleep_enable_timer_wakeup(5000000);
    esp_light_sleep_start();
    gpio_hold_dis(LED_STATUS);
    gpio_set_level(LED_STATUS, 0);
    ESP_LOGD("Pairing", "Stored Pairing data");
    esp_restart(); //restart the device after pairing
  }
}


#endif //_TWOMES_SENSOR_PAIRING_H