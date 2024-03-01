/**
 * @file subscriber.cpp
 * @author Braidan Duffy (legohead259@gmail.com)
 * 
 * @brief Adapted from the Micro-ROS Arduino Subscriber example
 * This script opens a serial transport through the Serial0 (`Serial`) port at 115200 baud.
 * When the Micro-ROS agent connects to this transport, the microcontroller will subscribe 
 * to the `micro_ros_arduino_subscriber` topic.
 * This topic is a `std_msgs.msg.Int32` type message that is expecting a `data` value of either
 * 0 or 1.
 * This value is written to the LED as a digital write, i.e. `data=0` is LED OFF, `data=1` is LED ON.
 * \\
 * On a ROS2 host with Micro-ROS installed in the workspace, open a terminal in the root workspace and enter the command:
 * 
 * ```
 * source install/setup.bash && ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0 -b 115200
 * ```
 * 
 * This will open a transport to the microcontroller.
 * You should see output reporting a successful creation of participant, topic, subscriber, and datareader.
 * Then, open a new terminal in the root workspace and enter the command:
 * 
 * ```
 * source install/setup.bash && ros2 topic pub /micro_ros_arduino_subscriber std_msgs/Int32 "{data: VALUE}"
 * ```
 * 
 * Where `VALUE` is replaced by either 0 or 1 depending on if you want the LED OFF or ON.
 * To exit the program, exit the terminal windows.
 * 
 * @version 1.0
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <Arduino.h>

#include <micro_ros_platformio.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

rcl_subscription_t subscriber;
std_msgs__msg__Int32 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


/**
 * @brief Enters an infinite loop where the LED blinks every 100ms
 * 
 */
void error_loop(){
    while(1){
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(100);
    }
}

/**
 * @brief Toggles the LED based on the value of the data field in the Int32 message.
 * This function is triggered when a new message is received on the `micro_ros_arduino_subscriber` topic.
 * 
 * @param msgin The message sent from the ROS2 host system via the `micro_ros_arduino_subscriber` topic
 */
void subscription_callback(const void * msgin) {  
    const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
    digitalWrite(LED_BUILTIN, (msg->data == 0) ? LOW : HIGH);  
}

/**
 * @brief This function is called when the microcontroller first boots.
 * It includes all of the initialization code to build the Micro-ROS transport and node
 * 
 */
void setup() {
    Serial.begin(115200);
        
    set_microros_serial_transports(Serial);
    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);  
    
    delay(2000);

    allocator = rcl_get_default_allocator();

    //create init_options
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    // create node
    RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

    // create subscriber
    RCCHECK(rclc_subscription_init_default(
        &subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "micro_ros_arduino_subscriber"));

    // create executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

/**
 * @brief This function is continually executed
 * 
 */
void loop() {
    delay(100);
    RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}