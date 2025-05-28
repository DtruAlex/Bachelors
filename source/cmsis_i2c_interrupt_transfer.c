/* Standard C Included Files */
#include <string.h>
#include <stdio.h>
/* SDK Included Files */
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "Driver_I2C.h"
#include "fsl_i2c_cmsis.h"
#include "pin_mux.h"

#define I2C_Address 0x55
#define BUFFER_SIZE 18   // 2 bytes (LED states) + 16 bytes (IP string) = 18 bytes total

extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
static volatile uint32_t I2C_Event;

// Buffer to store received data
static uint8_t rxBuffer[BUFFER_SIZE];
static uint32_t packetCounter = 0;

// Structure to represent the received data
typedef struct {
    uint8_t led1_state;         // 0 or 1
    uint8_t led2_state;         // 0 or 1
    char ip_string[17];         // IP address as string (16 chars + null terminator)
} ReceivedData;

uint32_t I2C1_GetFreq(void)
{
    return CLOCK_GetFreq(I2C1_CLK_SRC);
}

static void I2C_SignalEvent(uint32_t event)
{
    I2C_Event |= event;
}

// Function to display LED states with visual indication
void displayLEDStates(uint8_t led1, uint8_t led2)
{
    printf("LED States:\n");
    printf("  LED1: %s %s\n",
           led1 ? "ON " : "OFF",
           led1 ? "[●]" : "[○]");
    printf("  LED2: %s %s\n",
           led2 ? "ON " : "OFF",
           led2 ? "[●]" : "[○]");

    // Simple visual representation
    printf("  Visual: LED1 LED2\n");
    printf("          %s   %s\n",
           led1 ? " ● " : " ○ ",
           led2 ? " ● " : " ○ ");
}

// Function to parse and display IP address from string
void displayIPAddress(const char* ip_str)
{
    printf("IP Address: %s\n", ip_str);

    // Parse IP string to get individual octets for classification
    int ip1, ip2, ip3, ip4;
    int parsed = sscanf(ip_str, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);

    if (parsed == 4) {
        // Additional IP information
        if (ip1 == 192 && ip2 == 168) {
            printf("  Type: Private IP (Class C)\n");
        } else if (ip1 == 10) {
            printf("  Type: Private IP (Class A)\n");
        } else if (ip1 == 172 && ip2 >= 16 && ip2 <= 31) {
            printf("  Type: Private IP (Class B)\n");
        } else if (ip1 == 127) {
            printf("  Type: Loopback Address\n");
        } else if (strcmp(ip_str, "N/A") == 0) {
            printf("  Type: Not Connected\n");
        } else {
            printf("  Type: Public IP\n");
        }

        // Show in hex format if it's a valid IP
        if (strcmp(ip_str, "N/A") != 0) {
            printf("  Hex: 0x%02X.0x%02X.0x%02X.0x%02X\n", ip1, ip2, ip3, ip4);
        }
    } else {
        printf("  Type: Invalid/Special format\n");
    }
}

// Function to parse and display the complete received data
void parseAndDisplayData(uint8_t *buffer, uint32_t length)
{
    if (length < 2) {
        printf("Incomplete data received: %lu bytes (minimum 2 expected)\n", length);
        return;
    }

    printf("\n=== Packet #%lu ===\n", ++packetCounter);

    // Parse LED states (first 2 bytes)
    uint8_t led1_state = buffer[0];
    uint8_t led2_state = buffer[1];

    // Validate LED states (should be 0 or 1)
    if (led1_state > 1) {
        printf("WARNING: LED1 state invalid (%d), expected 0 or 1\n", led1_state);
        led1_state = led1_state ? 1 : 0;  // Normalize to 0 or 1
    }
    if (led2_state > 1) {
        printf("WARNING: LED2 state invalid (%d), expected 0 or 1\n", led2_state);
        led2_state = led2_state ? 1 : 0;  // Normalize to 0 or 1
    }

    // Display LED states
    displayLEDStates(led1_state, led2_state);
    printf("\n");

    // Parse IP address string (remaining bytes)
    if (length > 2) {
        // Create a buffer for the IP string with proper null termination
        char ip_string[17] = {0}; // 16 chars + null terminator

        // Copy the IP data, ensuring null termination
        uint32_t ip_length = length - 2;
        if (ip_length > 16) {
            ip_length = 16; // Limit to 16 characters
        }

        memcpy(ip_string, &buffer[2], ip_length);
        ip_string[16] = '\0'; // Ensure null termination

        // Remove any trailing null bytes or spaces for cleaner display
        for (int i = ip_length - 1; i >= 0; i--) {
            if (ip_string[i] == '\0' || ip_string[i] == ' ') {
                ip_string[i] = '\0';
            } else {
                break;
            }
        }

        // Display IP address
        displayIPAddress(ip_string);
    } else {
        printf("IP Address: Not received\n");
    }

    // Show raw data for debugging
    printf("\nRaw Data (%lu bytes): ", length);
    for (uint32_t i = 0; i < length; i++) {
        if (i < 2) {
            // Show LED data as decimal
            printf("%d ", buffer[i]);
        } else {
            // Show IP data as hex and ASCII
            if (buffer[i] >= 32 && buffer[i] <= 126) {
                printf("%02X('%c') ", buffer[i], buffer[i]);
            } else {
                printf("%02X ", buffer[i]);
            }
        }

        // Add line break every 8 bytes for readability
        if ((i + 1) % 8 == 0 && i < length - 1) {
            printf("\n          ");
        }
    }
    printf("\n");

    printf("========================================\n\n");
}

// Function to control actual LEDs if available
void controlLEDs(uint8_t led1_state, uint8_t led2_state)
{
    // TODO: Add actual LED control code here
    // Example:
    // GPIO_WritePinOutput(LED1_GPIO, LED1_PIN, led1_state);
    // GPIO_WritePinOutput(LED2_GPIO, LED2_PIN, led2_state);

    printf("LED Control Command: LED1=%s, LED2=%s\n",
           led1_state ? "ON" : "OFF",
           led2_state ? "ON" : "OFF");
}

/*!
 * @brief Main function
 */
int main(void)
{
    int32_t status;
    uint32_t bytesReceived;

    printf("I2C Slave - LED Control + IP Address Receiver (Updated)\n");
    printf("Slave Address: 0x%02X\n", I2C_Address);
    printf("Expected Data: 2 bytes (LED states) + 16 bytes (IP string) = %d bytes total\n", BUFFER_SIZE);
    printf("Data Format: [LED1][LED2][IP_STRING_16_BYTES]\n");
    printf("=====================================================\n\n");

    /* Initialize I2C peripheral */
    status = I2Cdrv->Initialize(I2C_SignalEvent);
    if (status != ARM_DRIVER_OK) {
        printf("I2C Initialize failed: %ld\n", status);
        return -1;
    }

    /* Power-on I2C peripheral */
    status = I2Cdrv->PowerControl(ARM_POWER_FULL);
    if (status != ARM_DRIVER_OK) {
        printf("I2C PowerControl failed: %ld\n", status);
        return -1;
    }

    /* Configure I2C bus - Set slave address */
    status = I2Cdrv->Control(ARM_I2C_OWN_ADDRESS, I2C_Address);
    if (status != ARM_DRIVER_OK) {
        printf("I2C Control (set address) failed: %ld\n", status);
        return -1;
    }

    printf("I2C Slave initialized successfully.\n");
    printf("Waiting for LED commands and IP address from ESP32 master...\n\n");

    while (1) {
        // Clear the buffer
        memset(rxBuffer, 0, BUFFER_SIZE);
        I2C_Event = 0;

        // Start slave receive operation
        status = I2Cdrv->SlaveReceive(rxBuffer, BUFFER_SIZE);
        if (status != ARM_DRIVER_OK) {
            printf("SlaveReceive failed: %ld\n", status);
            continue;
        }

        // Wait for transfer to complete
        while ((I2C_Event & ARM_I2C_EVENT_TRANSFER_DONE) == 0) {
            // Wait for completion
        }

        // Check for errors
        if (I2C_Event & ARM_I2C_EVENT_BUS_ERROR) {
            printf("Bus error detected!\n");
            I2C_Event &= ~ARM_I2C_EVENT_BUS_ERROR;
            continue;
        }

        // Get the actual number of bytes received
        bytesReceived = I2Cdrv->GetDataCount();

        if (bytesReceived > 0) {
            printf("Received %lu bytes\n", bytesReceived);

            // Parse and display the structured data
            parseAndDisplayData(rxBuffer, bytesReceived);

            // If we have LED data, control the LEDs
            if (bytesReceived >= 2) {
                controlLEDs(rxBuffer[0], rxBuffer[1]);
            }

            // Check if transfer was incomplete
            if (I2C_Event & ARM_I2C_EVENT_TRANSFER_INCOMPLETE) {
                printf("WARNING: Transfer was incomplete!\n");
                printf("Expected: %d bytes, Received: %lu bytes\n", BUFFER_SIZE, bytesReceived);
            }
        } else {
            printf("No data received in this transfer.\n");
        }

        // Clear transfer flags
        I2C_Event &= ~(ARM_I2C_EVENT_TRANSFER_DONE | ARM_I2C_EVENT_TRANSFER_INCOMPLETE | ARM_I2C_EVENT_GENERAL_CALL);
    }

    return 0;
}
