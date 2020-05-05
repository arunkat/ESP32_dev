// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// CAVEAT: This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style
// Checking of return codes and error values shall be omitted for brevity.  Please practice sound engineering practices
// when writing production code.

#include "provisioning.h"


#include "hsm_client_data.h"
#include "esp_task_wdt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iothub.h"
#include "azure_c_shared_utility/shared_util_options.h"
//#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xio.h"

#include "azure_prov_client/prov_device_client.h"
#include "azure_prov_client/prov_security_factory.h"

#include "parson.h"
#include <time.h>

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

//
// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
#include "iothubtransportmqtt_websockets.h"
#include "azure_prov_client/prov_transport_mqtt_ws_client.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
#include "iothubtransportamqp.h"
#include "azure_prov_client/prov_transport_amqp_client.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
#include "iothubtransportamqp_websockets.h"
#include "azure_prov_client/prov_transport_amqp_ws_client.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_http_client.h"
#endif // SAMPLE_HTTP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
#include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

typedef struct{
	char *deviceID;
	char *iothubURI;
	bool reg_complete;
} reg_device_context_t;

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void) { return NULL; }


static const char* global_prov_uri = "global.azure-devices-provisioning.net";
static const char* id_scope = "0ne000D965F";

//volatile bool g_registration_complete = false;
//static bool g_use_proxy = false;
//static const char* PROXY_ADDRESS = "127.0.0.1";

#define PROXY_PORT				  8888
#define MESSAGES_TO_SEND			2
#define TIME_BETWEEN_MESSAGES	   2

static void registration_status_callback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
	(void)user_context;
	(void)printf("Provisioning Status: %s\r\n", ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}
static void register_device_callback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
	if(user_context){
		reg_device_context_t *context = (reg_device_context_t*)user_context;
		if (register_result == PROV_DEVICE_RESULT_OK)
		{
			(void)printf("\r\nRegistration Information received from service: %s, deviceId: %s\r\n", iothub_uri, device_id);
		//	context->deviceID = (char *)malloc(strlen(device_id)+1);
		//	context->iothubURI = (char *)malloc(strlen(iothub_uri)+1);
		//	strcpy(context->deviceID, device_id);
		//	strcpy(context->iothubURI, iothub_uri);
		}
		else
		{
			(void)printf("\r\nFailure registering device: %s\r\n", ENUM_TO_STRING(PROV_DEVICE_RESULT, register_result));
		}
		context->reg_complete = true;
	} else {
		printf("user_context is null.\n");
	}
	printf("why do you still freeze here?\n");
}

//gets the device provision 
bool provision_get_from_file(const char *file, device_provision_t *prov)
{
	bool result = false;
	//FILE *fp = fopen(file, "r");
	//if(fp){
	//	fseek(fp, 0, SEEK_END); //seek the file pointer to the end of the file
	//	int size = ftell(fp); //this location is the size of the file. this is the esiest way to get a file's size in C.
	//	rewind(fp);//move the pointer back to the beginning. yes you have to do all this shit in C just for reading a file.
	//	char *buffer = malloc(size+1); //create buffer to store the file contents
	//	fread(buffer, 1, size, fp); // read contents into buffer
	//	fclose(fp); //close file
	//	buffer[size] = 0; //set the last char in buffer to NULL since we need a null terminated string.
	const char *json_str = "{\"registrationId\":\"hwLightABC01\",\"deviceType\":\"LightABC\",\"provisioned\":false,"
	"\"cert\":\"-----BEGIN CERTIFICATE-----\\nMIICwjCCAaqgAwIBAgIEOq0T2TANBgkqhkiG9w0BAQsFADATMREwDwYDVQQDDAhM\\naWdodEFCQzAeFw0yMDA0MTcxMjA2MzBaFw0yMTA0MTcxMjA2MzBaMBcxFTATBgNV\\nBAMMDGh3TGlnaHRBQkMwMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\\nAOYte6l7IAKjC61HNhwfbyd+wRp6z1JoU2cVD8QIUo1la+FODidT1fX2IoDyd2WF\\nIEPEYPRhmOh5U2tv52HE8sfsDS5KRi314PyjUUOrP9kIz2fRXPA9Kmj2HAqIapMu\\n+sqKl2iue4C9FGiKR64GGonarMmQX1CaB0PRwGbPXN9PFaIasn67iomE+F13Ujp/\\nLsNQTVpFM/g+QwdfhP3rEfUQLIZIN6Bf4WkojQhLWNz5PznT9irIh1AB303mbJK4\\nGrK5i3nfqq+Iwne2kaGcAUD9UtQaO6NB9zM2kFzdWDbZaa6OiUGPu9Tex+81/pps\\nITcV+QFjjMmZFJVYwLWxdWcCAwEAAaMaMBgwFgYDVR0lAQH/BAwwCgYIKwYBBQUH\\nAwIwDQYJKoZIhvcNAQELBQADggEBADOnKPcm48473noxTbzMkfQKDWtKK91k1E4N\\ndiRbVLSGYU8AuxyPqV4/dgzJElrMVH5YzKw6Ar3K5lpoHprsjkhD8Jsr2WAfWTiw\\ndyUH6yxJvSiaUJhEKy6X5rHa8gJ5NqRQZZOvaFikVJh3uMBK7a9OJ3yq+yBPyZC0\\nb6dfAQSsjuFHu8jasbOr6zzA1s3TYluauFWoZsyAJJHISefVKc7N3PqfFru0sc/2\\nnYi5fD9OxPOHbrBLda81Nh339/EblgbwnWnGEZOmQZgSRgLkZloYBcwlTnzbpnAM\\npLEf5Ju3g2zwjBACrRfaIjz0d8+ZkNcnK9RYbqc8N2+vFYCsDEA=\\n-----END CERTIFICATE-----\","
	"\"key\":\"-----BEGIN RSA PRIVATE KEY-----\\nMIIEpAIBAAKCAQEA5i17qXsgAqMLrUc2HB9vJ37BGnrPUmhTZxUPxAhSjWVr4U4O\\nJ1PV9fYigPJ3ZYUgQ8Rg9GGY6HlTa2/nYcTyx+wNLkpGLfXg/KNRQ6s/2QjPZ9Fc\\n8D0qaPYcCohqky76yoqXaK57gL0UaIpHrgYaidqsyZBfUJoHQ9HAZs9c308Vohqy\\nfruKiYT4XXdSOn8uw1BNWkUz+D5DB1+E/esR9RAshkg3oF/haSiNCEtY3Pk/OdP2\\nKsiHUAHfTeZskrgasrmLed+qr4jCd7aRoZwBQP1S1Bo7o0H3MzaQXN1YNtlpro6J\\nQY+71N7H7zX+mmwhNxX5AWOMyZkUlVjAtbF1ZwIDAQABAoIBAQC87HIGjn+cinTI\\nGZ3pAUf7k8ctU8Wc7vIdtqTFEsunMKqWN7nYP7Bq/EYfrmOfWOA9nw6xJvYZQZPd\\np/CzR7K5sx6yctYdXSX4VpgZwZJbMicCIE53BM0tb2tenc9T1QiVe6GAk03dQdRh\\nZbYluO7JXUna+vuwrWvvF1cjS2oAAltfr8BJoFIOAFbhd/wOXesPLgHwI2LJvC61\\nvfGtNrGlNqeUE0Q/muIs8eXzBHQQtpd9GLmTIh6gaFSzUVXaI2I7nn8uVSegraPK\\n+kmhHp3fgpEDHP70SdsE4C8EgRTSWadbzhMOuQcMalFIhqrCPbqaAUpUTsog07R0\\niN40s69BAoGBAP6hdA3KhUAFGZ/xhKDw/RQUXzdGx/jDVk727Jce5wU1OViC9BSv\\nqKGsqxeIGmNLX2oSAE+0feczLNPVh/KuCeNqa2gs5EaE6cRQk+bjAvliNytVLBpL\\nzorxAp2Rc0jIlYm+dtwidvacxAruORCBr0wMDo/RANjz07GzcoFi8miHAoGBAOdq\\nXZcjeAV4Rt3zSidkv/FXDwtqN/UQyffytemRcJLYCvc8oTM1LPtskuslyMkc1idu\\n9MaLVBVBwQSK4wNg39ybLEqsjwGfxXMGgGBwFHlQT61glfPDA3jAr6iWI8jzRGKn\\n0Vw9kbr00ByAfaxrr/5Dw0gr/z5UFHfHXFyHCSQhAoGARJFVnyEaINM+w0NWY8CB\\nZhbWTRxSXTq80ybLLyazL0PV3W/mKmvjDSZiLEQKVxLE7ttKGiyQeuHdAG5P3Zng\\nL81IfxUXo6XHDYZlTZd0BZPdJ14YMjyXsfKUsbmpQcBCBIW1nDHrtx0f7ZGY7Ej/\\n24qjoTa287U1HHUmMJFklaECgYEAv4dQGIP5lQVcGdx/FiWTmwpD4F20HHcdwcI2\\nfy6pbk+ym7epbzlmllzhKA+oo5LjR9XUbvLnz4QRXVIZ2zT1cp9XRCKXZW+3uqC5\\n5Zc9yr4Gg+d5lDtmBy3q9Gv3CB0XD1P3uhEXKRXvnHdYDDlAev/Yg0YuxYZPPmdY\\n8ReuICECgYBqZb+9VrA5S58KJ23n7YftLULySLDygkqkbAE6pP4yjaUJB7MM42BP\\nwJGvlImwQH3ibEqapXdt7y73R8zVLvFB2HfrVI4IOpuOdHxsv5EtWk63otQwhSE0\\nNmw8jRDTqHu044pj+3s5FtjGPS48o4BxJMbaZ3eQ7xVQU3Loa/WfbA==\\n-----END RSA PRIVATE KEY-----\","
	"\"deviceId\":\"\",\"hubName\":\"\"}";
		printf("%s\n", json_str);
		//now convert the JSON string into the provision object.
		memset(prov, 0, sizeof(device_provision_t));
		JSON_Value* root_value = json_parse_string(json_str);
		if(root_value){
			JSON_Object* root_object = json_value_get_object(root_value);
			JSON_Value* registrationIdVal = json_object_get_value(root_object, "registrationId");
			JSON_Value* deviceTypeVal = json_object_get_value(root_object, "deviceType");
			JSON_Value* provisionedVal = json_object_get_value(root_object, "provisioned");
			JSON_Value* certVal = json_object_get_value(root_object, "cert");
			JSON_Value* keyVal = json_object_get_value(root_object, "key");
			JSON_Value* deviceIdVal = json_object_get_value(root_object, "deviceId");
			JSON_Value* hubNameVal = json_object_get_value(root_object, "hubName");

			
			bool missingRequired = false;
			if (registrationIdVal){
				const char* tmp = json_value_get_string(registrationIdVal);
				prov->registrationId = malloc(strlen(tmp)+1);
				strcpy(prov->registrationId, tmp);
			} else {
				missingRequired = true;
				printf("Missing registrationId.\n");
			}
			if (deviceTypeVal){
				const char* tmp = json_value_get_string(deviceTypeVal);
				prov->deviceType = malloc(strlen(tmp)+1);
				strcpy(prov->deviceType, tmp);
			} else {
				missingRequired = true;
				printf("Missing deviceType.\n");
			}
			if (provisionedVal){
				prov->provisioned = json_value_get_boolean(provisionedVal);
			} else {
				prov->provisioned = false;
			}
			if (certVal){
				const char* e = json_value_get_string(certVal);
				prov->cert = malloc(strlen(e)+1);
				strcpy(prov->cert, e);
			} else {
				missingRequired = true;
				printf("Missing certificate.\n");
			}
			if (keyVal){
				const char* tmp = json_value_get_string(keyVal);
				prov->key = malloc(strlen(tmp)+1);
				strcpy(prov->key, tmp);
			} else {
				missingRequired = true;
				printf("Missing private key.\n");
			}
			if (deviceIdVal){
				const char* tmp = json_value_get_string(deviceIdVal);
				prov->deviceId = malloc(strlen(tmp)+1);
				strcpy(prov->deviceId, tmp);
			} else {
				prov->provisioned = false;
			}
			if (hubNameVal){
				const char* tmp = json_value_get_string(hubNameVal);
				prov->hubName = malloc(strlen(tmp)+1);
				strcpy(prov->hubName, tmp);
			} else {
				prov->provisioned = false;
			}
			result = !missingRequired;
		} else {
			printf("Bad JSON file. check syntax.\n");
		}
	//}
	return result;
}

bool provision_save_to_file(const char *file, const device_provision_t *prov)
{
	/*bool result = false;
	FILE *fp = fopen(file, "w");
	if(fp){
		result = true;

		char* str;

		JSON_Value* root_value = json_value_init_object();
		JSON_Object* root_object = json_value_get_object(root_value);

		// Only reported properties:
		json_object_set_string(root_object, "registrationId", prov->registrationId);
		json_object_set_string(root_object, "deviceType", prov->deviceType);
		json_object_set_boolean(root_object, "provisioned", prov->provisioned);
		json_object_set_string(root_object, "cert", prov->cert);
		json_object_set_string(root_object, "key", prov->key);
		json_object_set_string(root_object, "deviceId", prov->deviceId);
		json_object_set_string(root_object, "hubName", prov->hubName);

		str = json_serialize_to_string(root_value);
		json_value_free(root_value);

		fwrite(str, 1, strlen(str), fp);
		fclose(fp);
		free(str);

	}
	return result;
	*/
	return true;
}

PROV_DEVICE_RESULT provision_device(char** deviceID, char** iothubURI)
{
	
	esp_task_wdt_add(NULL);
	esp_task_wdt_reset();
	PROV_DEVICE_RESULT result;
	SECURE_DEVICE_TYPE hsm_type;
	//hsm_type = SECURE_DEVICE_TYPE_TPM;
	hsm_type = SECURE_DEVICE_TYPE_X509;
	//hsm_type = SECURE_DEVICE_TYPE_SYMMETRIC_KEY;

	// Used to initialize IoTHub SDK subsystem
	(void)prov_dev_security_init(hsm_type);

	// Set the symmetric key if using they auth type
	//prov_dev_set_symmetric_key_info("<symm_registration_id>", "<symmetric_Key>");

	//HTTP_PROXY_OPTIONS http_proxy;
	PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport;
	reg_device_context_t deviceContext;
	memset(&deviceContext, 0, sizeof(reg_device_context_t));

	//memset(&http_proxy, 0, sizeof(HTTP_PROXY_OPTIONS));

	// Protocol to USE - HTTP, AMQP, AMQP_WS, MQTT, MQTT_WS
#ifdef SAMPLE_MQTT
	prov_transport = Prov_Device_MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
	prov_transport = Prov_Device_MQTT_WS_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
	prov_transport = Prov_Device_AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
	prov_transport = Prov_Device_AMQP_WS_Protocol;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
	prov_transport = Prov_Device_HTTP_Protocol;
#endif // SAMPLE_HTTP

	printf("Provisioning API Version: %s\r\n", Prov_Device_LL_GetVersionString());

	PROV_DEVICE_RESULT prov_device_result = PROV_DEVICE_RESULT_ERROR;
	PROV_DEVICE_LL_HANDLE prov_device_handle;
	printf("creating handle\n");
	prov_device_handle = Prov_Device_LL_Create(global_prov_uri, id_scope, prov_transport);
	printf("handle created\n");
	if ((prov_device_handle = Prov_Device_LL_Create(global_prov_uri, id_scope, prov_transport)) == NULL)
	{
		(void)printf("failed calling Prov_Device_Create\r\n");
	}
	else
	{
		esp_task_wdt_reset();
		/*

		//bool traceOn = true;
		//Prov_Device_SetOption(prov_device_handle, PROV_OPTION_LOG_TRACE, &traceOn);
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
		// Setting the Trusted Certificate.  This is only necessary on system with without
		// built in certificate stores.
		Prov_Device_SetOption(prov_device_handle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES
*/
		// This option sets the registration ID it overrides the registration ID that is 
		// set within the HSM so be cautious if setting this value
		printf("Set registration ID\n");
		Prov_Device_LL_SetOption(prov_device_handle, PROV_REGISTRATION_ID, "hwLightABC01");
		printf("Registering Device.");
		time_t start = time(NULL);
		printf("time: %d", (int)start);
		if(Prov_Device_LL_Register_Device(prov_device_handle, register_device_callback, &deviceContext, registration_status_callback, &deviceContext) == PROV_DEVICE_RESULT_OK){
			do {
				
				esp_task_wdt_reset();
				Prov_Device_LL_DoWork(prov_device_handle);
				ThreadAPI_Sleep(10);
			} while (!deviceContext.reg_complete && (start + 10 > time(NULL)));
		}
		
		esp_task_wdt_reset();
		printf("you should continue here.");
		*deviceID = deviceContext.deviceID;
		*iothubURI = deviceContext.iothubURI;

		Prov_Device_LL_Destroy(prov_device_handle);
	}
	prov_dev_security_deinit();
/*
	// Free all the sdk subsystem
	IoTHub_Deinit();

	(void)printf("Press enter key to exit:\r\n");
	(void)getchar;
*/
	//return prov_device_result;
	return PROV_DEVICE_RESULT_ERROR;
}
