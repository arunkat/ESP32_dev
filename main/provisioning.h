#ifndef PROVISIONING_H
#define PROVISIONING_H


#include "azure_prov_client/prov_device_client.h"


typedef struct{
    char *registrationId;//": "hwLightABC01",
    char *deviceType;//": "LightABC",
    bool provisioned;//": true,
    char *cert;//": "-----BEGIN CERTIFICATE-----\nMIICwjCCAaqgAwIBAgIEOq0T2TANBgkqhkiG9w0BAQsFADATMREwDwYDVQQDDAhM\naWdodEFCQzAeFw0yMDA0MTcxMjA2MzBaFw0yMTA0MTcxMjA2MzBaMBcxFTATBgNV\nBAMMDGh3TGlnaHRBQkMwMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\nAOYte6l7IAKjC61HNhwfbyd+wRp6z1JoU2cVD8QIUo1la+FODidT1fX2IoDyd2WF\nIEPEYPRhmOh5U2tv52HE8sfsDS5KRi314PyjUUOrP9kIz2fRXPA9Kmj2HAqIapMu\n+sqKl2iue4C9FGiKR64GGonarMmQX1CaB0PRwGbPXN9PFaIasn67iomE+F13Ujp/\nLsNQTVpFM/g+QwdfhP3rEfUQLIZIN6Bf4WkojQhLWNz5PznT9irIh1AB303mbJK4\nGrK5i3nfqq+Iwne2kaGcAUD9UtQaO6NB9zM2kFzdWDbZaa6OiUGPu9Tex+81/pps\nITcV+QFjjMmZFJVYwLWxdWcCAwEAAaMaMBgwFgYDVR0lAQH/BAwwCgYIKwYBBQUH\nAwIwDQYJKoZIhvcNAQELBQADggEBADOnKPcm48473noxTbzMkfQKDWtKK91k1E4N\ndiRbVLSGYU8AuxyPqV4/dgzJElrMVH5YzKw6Ar3K5lpoHprsjkhD8Jsr2WAfWTiw\ndyUH6yxJvSiaUJhEKy6X5rHa8gJ5NqRQZZOvaFikVJh3uMBK7a9OJ3yq+yBPyZC0\nb6dfAQSsjuFHu8jasbOr6zzA1s3TYluauFWoZsyAJJHISefVKc7N3PqfFru0sc/2\nnYi5fD9OxPOHbrBLda81Nh339/EblgbwnWnGEZOmQZgSRgLkZloYBcwlTnzbpnAM\npLEf5Ju3g2zwjBACrRfaIjz0d8+ZkNcnK9RYbqc8N2+vFYCsDEA=\n-----END CERTIFICATE-----",
    char *key;//": "-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA5i17qXsgAqMLrUc2HB9vJ37BGnrPUmhTZxUPxAhSjWVr4U4O\nJ1PV9fYigPJ3ZYUgQ8Rg9GGY6HlTa2/nYcTyx+wNLkpGLfXg/KNRQ6s/2QjPZ9Fc\n8D0qaPYcCohqky76yoqXaK57gL0UaIpHrgYaidqsyZBfUJoHQ9HAZs9c308Vohqy\nfruKiYT4XXdSOn8uw1BNWkUz+D5DB1+E/esR9RAshkg3oF/haSiNCEtY3Pk/OdP2\nKsiHUAHfTeZskrgasrmLed+qr4jCd7aRoZwBQP1S1Bo7o0H3MzaQXN1YNtlpro6J\nQY+71N7H7zX+mmwhNxX5AWOMyZkUlVjAtbF1ZwIDAQABAoIBAQC87HIGjn+cinTI\nGZ3pAUf7k8ctU8Wc7vIdtqTFEsunMKqWN7nYP7Bq/EYfrmOfWOA9nw6xJvYZQZPd\np/CzR7K5sx6yctYdXSX4VpgZwZJbMicCIE53BM0tb2tenc9T1QiVe6GAk03dQdRh\nZbYluO7JXUna+vuwrWvvF1cjS2oAAltfr8BJoFIOAFbhd/wOXesPLgHwI2LJvC61\nvfGtNrGlNqeUE0Q/muIs8eXzBHQQtpd9GLmTIh6gaFSzUVXaI2I7nn8uVSegraPK\n+kmhHp3fgpEDHP70SdsE4C8EgRTSWadbzhMOuQcMalFIhqrCPbqaAUpUTsog07R0\niN40s69BAoGBAP6hdA3KhUAFGZ/xhKDw/RQUXzdGx/jDVk727Jce5wU1OViC9BSv\nqKGsqxeIGmNLX2oSAE+0feczLNPVh/KuCeNqa2gs5EaE6cRQk+bjAvliNytVLBpL\nzorxAp2Rc0jIlYm+dtwidvacxAruORCBr0wMDo/RANjz07GzcoFi8miHAoGBAOdq\nXZcjeAV4Rt3zSidkv/FXDwtqN/UQyffytemRcJLYCvc8oTM1LPtskuslyMkc1idu\n9MaLVBVBwQSK4wNg39ybLEqsjwGfxXMGgGBwFHlQT61glfPDA3jAr6iWI8jzRGKn\n0Vw9kbr00ByAfaxrr/5Dw0gr/z5UFHfHXFyHCSQhAoGARJFVnyEaINM+w0NWY8CB\nZhbWTRxSXTq80ybLLyazL0PV3W/mKmvjDSZiLEQKVxLE7ttKGiyQeuHdAG5P3Zng\nL81IfxUXo6XHDYZlTZd0BZPdJ14YMjyXsfKUsbmpQcBCBIW1nDHrtx0f7ZGY7Ej/\n24qjoTa287U1HHUmMJFklaECgYEAv4dQGIP5lQVcGdx/FiWTmwpD4F20HHcdwcI2\nfy6pbk+ym7epbzlmllzhKA+oo5LjR9XUbvLnz4QRXVIZ2zT1cp9XRCKXZW+3uqC5\n5Zc9yr4Gg+d5lDtmBy3q9Gv3CB0XD1P3uhEXKRXvnHdYDDlAev/Yg0YuxYZPPmdY\n8ReuICECgYBqZb+9VrA5S58KJ23n7YftLULySLDygkqkbAE6pP4yjaUJB7MM42BP\nwJGvlImwQH3ibEqapXdt7y73R8zVLvFB2HfrVI4IOpuOdHxsv5EtWk63otQwhSE0\nNmw8jRDTqHu044pj+3s5FtjGPS48o4BxJMbaZ3eQ7xVQU3Loa/WfbA==\n-----END RSA PRIVATE KEY-----",
    char *deviceId;//": "hwLightABC01",
    char *hubName;//": "eraniothub1.azure-devices.net"
} device_provision_t;

bool provision_get_from_file(const char *file, device_provision_t *prov);
bool provision_save_to_file(const char *file, const device_provision_t *prov);

PROV_DEVICE_RESULT provision_device(char** deviceID, char** iothub_uri);

#endif