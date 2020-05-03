// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This sample shows how to translate the Device Twin json received from Azure IoT Hub into meaningful data for your application.
// It uses the parson library, a very lightweight json parser.

// There is an analogous sample using the serializer - which is a library provided by this SDK to help parse json - in devicetwin_simplesample.
// Most applications should use this sample, not the serializer.

// WARNING: Check the return of all API calls when developing your solution. Return checks ommited for sample simplification.

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub.h"
#include "iothub_message.h"
#include "parson.h"
#include "sdkconfig.h"

#include "lightABCdevice.h"
#include "device.h"
#include "provisioning.h"

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
    #include "iothubtransportmqtt.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    #include "iothubtransportmqtt_websockets.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    #include "iothubtransportamqp.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    #include "iothubtransportamqp_websockets.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    #include "iothubtransporthttp.h"
#endif // SAMPLE_HTTP

/* Paste in the your iothub device connection string  */

extern const char* HSM_CERTIFICATE;
extern const char* HSM_PRIVATE_KEY;

IOTHUB_DEVICE_CLIENT_LL_HANDLE *handle;

#define DOWORK_LOOP_NUM     3


//  Converts the light object into a JSON blob with reported properties that is ready to be sent across the wire as a twin.


static const char* connectionStringFormat = "HostName=%s;DeviceId=%s;x509=true";
extern char* serializeToJson(void* props);
extern bool parseFromJson(const char* json, bool twin_update_state, void* props);
extern void deviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback);

//extern void reportedStateCallback(int status_code, void* userContextCallback);

extern int deviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback);



void iothub_client_device_twin_and_methods_run(device_provision_t *provisioningStatus)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
    IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle;
    handle = &iotHubClientHandle;
    device_t device;
    device._handle = &iotHubClientHandle;
    device.properties = lightABC_AllocateProps();

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    protocol = MQTT_WebSocket_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    protocol = AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    protocol = AMQP_Protocol_over_WebSocketsTls;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    protocol = HTTP_Protocol;
#endif // SAMPLE_HTTP

    char *connectionString = malloc(strlen(connectionStringFormat) + strlen(provisioningStatus->hubName) + strlen(provisioningStatus->deviceId)+1);
    sprintf(connectionString, connectionStringFormat, provisioningStatus->hubName, provisioningStatus->deviceId);
	if ((iotHubClientHandle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol)) == NULL)
	{
		printf("ERROR: iotHubClientHandle is NULL!\r\n");
	}
	else
	{
		// Uncomment the following lines to enable verbose logging (e.g., for debugging).
		//bool traceOn = true;
		//(void)IoTHubDeviceClient_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn);
		if((IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_X509_CERT, HSM_CERTIFICATE) != IOTHUB_CLIENT_OK) ||
		(IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_X509_PRIVATE_KEY, HSM_PRIVATE_KEY) != IOTHUB_CLIENT_OK))
		{
			(void)printf("failure to set option \"TrustedCerts\"\r\n");
		}

		//LightABC_t Light;
		//memset(&Light, 0, sizeof(LightABC_t));

		char* reportedProperties = serializeToJson(device.properties);
		(void)IoTHubDeviceClient_LL_SendReportedState(iotHubClientHandle, (const unsigned char*)reportedProperties, strlen(reportedProperties), reportedStateCallback, NULL);
		(void)IoTHubDeviceClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, &device);
		(void)IoTHubDeviceClient_LL_SetDeviceTwinCallback(iotHubClientHandle, deviceTwinCallback, &device);

		while (1) {
			IoTHubDeviceClient_LL_DoWork(iotHubClientHandle);
			ThreadAPI_Sleep(10);
		}

		IoTHubDeviceClient_LL_Destroy(iotHubClientHandle);
		free(reportedProperties);
		free(device.properties);
	}
	
    free(connectionString);
}


void iothub_device_init()
{
	if (IoTHub_Init() != 0)
    {
        printf("Failed to initialize the platform.\r\n");
    } else {
        
        printf("Getting provision status.\n");
        bool device_ready = false;
        device_provision_t provisioningStatus;
        if(provision_get_from_file("", &provisioningStatus)){
            //this is temporary
            HSM_CERTIFICATE = provisioningStatus.cert;
            HSM_PRIVATE_KEY = provisioningStatus.key;
            if(provisioningStatus.provisioned){
                printf("Device provisioned.\n");
                device_ready = true;
            } else {
                printf("Device not provisioned.\n");
                if(provisioningStatus.deviceId)    free(provisioningStatus.deviceId);
                if(provisioningStatus.hubName)    free(provisioningStatus.hubName);
                if(provision_device(&provisioningStatus.deviceId, &provisioningStatus.hubName) == PROV_DEVICE_RESULT_OK){
                    provisioningStatus.provisioned = true;
                    provision_save_to_file("", &provisioningStatus);
                    printf("Device provisioned successfully.");
                    device_ready = true;

                } else {
                    printf("Device failed to provision.");
                }

            }
        } else {
            printf("Failed to get provision status.\n");
        }

        if(device_ready){
            iothub_client_device_twin_and_methods_run(&provisioningStatus);
        }
    }

    
    IoTHub_Deinit();
}