/*
*
*   ===================================================
*       CropDrop Bot (CB) Theme [eYRC 2025-26]
*   ===================================================
*
*  This script is intended to be an Boilerplate for 
*  Task 1A of CropDrop Bot (CB) Theme [eYRC 2025-26].
*
*  Filename:		task1A.c
*  Created:		    10/10/2025
*  Last Modified:	15/10/2025
*  Author:		    e-Yantra Team
*  Team ID:		    [ CB_2202 ]
*  This software is made available on an "AS IS WHERE IS BASIS".
*  Licensee/end user indemnifies and will keep e-Yantra indemnified from
*  any and all claim(s) that emanate from the use of the Software or
*  breach of the terms of this agreement.
*  
*  e-Yantra - An MHRD project under National Mission on Education using ICT (NMEICT)
*
*****************************************************************************************
*/


#include "coppeliasim_client.h"  // Include our header

// Global client instance for socket communication
SocketClient client;

// ----------------------
// Forward declarations (these will move to header gradually)
// ----------------------
void* control_loop(void* arg);          // Only control_loop remains

/**
 * @brief Establishes connection to the CoppeliaSim server
 * @param c Pointer to SocketClient structure
 * @param ip IP address of the server (typically "127.0.0.1" for localhost)
 * @param port Port number of the server (typically 50002)
 * @return 1 if connection successful, 0 if failed
 */

int connect_to_server(SocketClient* c, const char* ip, int port) {
#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 0;
    }
#endif
    
    // Create TCP socket
    c->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (c->sock < 0) {
        printf("Socket creation failed\n");
        return 0;
    }

    // Setup server address structure
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    // Attempt to connect to server
    if (connect(c->sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        CLOSESOCKET(c->sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 0;
    }

    c->running = true;

    // Start the receive thread to handle incoming sensor data
#ifdef _WIN32
    c->recv_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)receive_loop, c, 0, NULL);
#else
    pthread_create(&c->recv_thread, NULL, receive_loop, c);
#endif

    return 1;
}
/*===============================================================================================*/
/**
 * @brief Main control loop thread for robot behavior
 * @param arg Pointer to SocketClient structure (cast from void*)
 * @return NULL when thread exits
 * 
 * This is where you should implement your robot's control logic.
 * The function runs continuously while the client is connected.
 * 
 * Available functions for control:
 * - set_motor(c, left_speed, right_speed): Control motor speeds
 * - Access sensor data via: c->sensor_values[index] and c->sensor_count
 */
void* control_loop(void* arg) {
    SocketClient* c = (SocketClient*)arg;
    
    // Ideal sensor values (when robot is perfectly centered)
    float ideal_sensors[5] = {0.093, 0.826, 0.826, 0.826, 0.093};
    float Kp = 0.8f;             // proportional gain
    float forward_speed = 0.5f;  // base forward speed

    // Weights: left = -2 ... right = +2
    float weights[5] = {-2.0f, -1.0f, 0.0f, 1.0f, 2.0f};

    while (c->running) {
        if (c->sensor_count < 5) {
            SLEEP(20);
            continue;  // wait until all 5 sensor readings are available
        }

        // Compute weighted error using ideal vs actual
        float num = 0.0f, den = 0.0f;
        for (int i = 0; i < 5; i++) {
            float error_i = ideal_sensors[i] - c->sensor_values[i];
            num += error_i * weights[i];
            den += (error_i < 0 ? -error_i : error_i);  // magnitude of error as weight
        }

        float error = 0.0f;
        if (den > 1e-3f) {
            error = num / den;  // error around 0 when centered
        }

        // Proportional correction
        float correction = Kp * error;

        // Apply correction to motor speeds
        float left_speed = forward_speed - correction;
        float right_speed = forward_speed + correction;

        // Clamp motor speeds
        if (left_speed > 1.0f) left_speed = 1.0f;
        if (left_speed < 0.0f) left_speed = 0.0f;
        if (right_speed > 1.0f) right_speed = 1.0f;
        if (right_speed < 0.0f) right_speed = 0.0f;

        // Apply motor commands
        set_motor(c, left_speed, right_speed);

        SLEEP(20);  // 50Hz update
    }

    // Stop motors when exiting
    set_motor(c, 0.0f, 0.0f);
    return NULL;
}
/*===============================================================================================*/

/**
 * @brief Main function - Entry point of the program
 * @return 0 if successful, -1 if connection failed
 * 
 * This function:
 * 1. Connects to the CoppeliaSim server
 * 2. Starts the control thread for robot behavior
 * 3. Continuously displays sensor data
 * 4. Handles cleanup when program exits
 */
int main() {
    if (!connect_to_server(&client, "127.0.0.1", 50002)) {
        printf("Failed to connect to CoppeliaSim server. Make sure:\n");
        printf("1. CoppeliaSim is running\n");
        printf("2. The simulation scene is loaded\n");
        printf("3. The ZMQ remote API is enabled on port 50002\n");
        return -1;
    }
    
    printf("Successfully connected to CoppeliaSim server!\n");
    printf("Starting control thread...\n");
    
    // Start the control thread for robot behavior
#ifdef _WIN32
    client.control_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)control_loop, &client, 0, NULL);
#else
    pthread_create(&client.control_thread, NULL, control_loop, &client);
#endif

    // Main loop: Display sensor data continuously
    printf("Monitoring sensor data... (Press Ctrl+C to exit)\n");
    while (1) {
        if (client.sensor_count > 0) {
            printf("Sensors (%d): ", client.sensor_count);
            for (int i = 0; i < client.sensor_count; i++) {
                printf("%.3f ", client.sensor_values[i]);
            }
            printf("\n");
        } else {
            printf("Waiting for sensor data...\n");
        }
        
        SLEEP(200);  // Update display every 200ms
    }
    printf("Disconnecting...\n");
    disconnect(&client);
    return 0;
}