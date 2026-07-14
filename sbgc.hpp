/* 
	SimpleBGC Serial API  library
	More info: http://www.basecamelectronics.com

  * last edit date: 19.08.2015
  * compatible with the revision 2.4, 2.5 of Serial API specification.

	Copyright (c) 2014-2015 Aleksei Moskalenko
	All rights reserved.
	
	
  This software is free for use and redistribution under a BSD license:
   
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	    * Redistributions of source code must retain the above copyright
	      notice, this list of conditions and the following disclaimer.
	    * Redistributions in binary form must reproduce the above copyright
	      notice, this list of conditions and the following disclaimer in the
	      documentation and/or other materials provided with the distribution.
	    * Neither the name of the Basecamelectronics company nor the
	      names of its contributors may be used to endorse or promote products
	      derived from this software without specific prior written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/   

#ifndef  __SBGC__
#define  __SBGC__

// !!!!Define the endianess of your machine!!!! 
//#define SYS_LITTLE_ENDIAN

// !!!!Uncomment only if structures are packed (use it for 8bit machines to save some program space)
//#define SYS_STRUCT_PACKED


#include <inttypes.h>

#define SBGC_CMD_START_BYTE 0x24

// Size of header and checksums
#define SBGC_CMD_NON_PAYLOAD_BYTES 6
// Max. size of a command after packing to bytes
#define SBGC_CMD_MAX_BYTES 255
// Max. size of a payload data
#define SBGC_CMD_DATA_SIZE (SBGC_CMD_MAX_BYTES - SBGC_CMD_NON_PAYLOAD_BYTES)



////////////////////// Command ID definitions ////////////////
#define SBGC_CMD_READ_PARAMS  82
#define SBGC_CMD_WRITE_PARAMS  87
#define SBGC_CMD_REALTIME_DATA  68
#define SBGC_CMD_BOARD_INFO  86
#define SBGC_CMD_CALIB_ACC  65
#define SBGC_CMD_CALIB_GYRO  103
#define SBGC_CMD_CALIB_EXT_GAIN  71
#define SBGC_CMD_USE_DEFAULTS  70
#define SBGC_CMD_CALIB_POLES  80
#define SBGC_CMD_RESET  114
#define SBGC_CMD_HELPER_DATA 72
#define SBGC_CMD_CALIB_OFFSET  79
#define SBGC_CMD_CALIB_BAT  66
#define SBGC_CMD_MOTORS_ON   77
#define SBGC_CMD_MOTORS_OFF  109
#define SBGC_CMD_CONTROL   67
#define SBGC_CMD_TRIGGER_PIN  84
#define SBGC_CMD_EXECUTE_MENU 69
#define SBGC_CMD_GET_ANGLES  73
#define SBGC_CMD_CONFIRM  67


// Starting from board ver.3.0
#define SBGC_CMD_BOARD_INFO_3  20
#define SBGC_CMD_READ_PARAMS_3 21
#define SBGC_CMD_WRITE_PARAMS_3 22
#define SBGC_CMD_REALTIME_DATA_3  23
#define SBGC_CMD_SELECT_IMU_3 24
#define SBGC_CMD_REALTIME_DATA_4  25
#define SBGC_CMD_ENCODERS_CALIB_OFFSET_4  26
#define SBGC_CMD_ENCODERS_CALIB_FLD_OFFSET_4 27
#define SBGC_CMD_READ_PROFILE_NAMES 28
#define SBGC_CMD_WRITE_PROFILE_NAMES 29

#define SBGC_CMD_QUEUE_PARAMS_INFO_3 30
#define SBGC_CMD_SET_ADJ_VARS_VAL 31
#define SBGC_CMD_SAVE_PARAMS_3 32
#define SBGC_CMD_READ_PARAMS_EXT 33
#define SBGC_CMD_WRITE_PARAMS_EXT 34
#define SBGC_CMD_AUTO_PID 35
#define SBGC_CMD_SERVO_OUT 36
#define SBGC_CMD_BODE_TEST_START_STOP 37
#define SBGC_CMD_BODE_TEST_DATA 38
#define SBGC_CMD_I2C_WRITE_REG_BUF 39
#define SBGC_CMD_I2C_READ_REG_BUF 40
#define SBGC_CMD_WRITE_EXTERNAL_DATA 41
#define SBGC_CMD_READ_EXTERNAL_DATA 42
#define SBGC_CMD_READ_ADJ_VARS_CFG 43
#define SBGC_CMD_WRITE_ADJ_VARS_CFG 44
#define SBGC_CMD_API_VIRT_CH_CONTROL 45
#define SBGC_CMD_ADJ_VARS_STATE 46
#define SBGC_CMD_EEPROM_WRITE 47
#define SBGC_CMD_EEPROM_READ 48
#define SBGC_CMD_CALIB_INFO 49
#define SBGC_CMD_SIGN_MESSAGE_3 50
#define SBGC_CMD_BOOT_MODE_3 51
#define SBGC_CMD_SYSTEM_STATE 52
#define SBGC_CMD_READ_FILE 53
#define SBGC_CMD_WRITE_FILE 54
#define SBGC_CMD_FS_CLEAR_ALL 55
#define SBGC_CMD_AHRS_HELPER 56
#define SBGC_CMD_RUN_SCRIPT 57
#define SBGC_CMD_SCRIPT_DEBUG 58
#define SBGC_CMD_CALIB_MAG 59
#define SBGC_CMD_UART_BYPASS 60
#define SBGC_CMD_GET_ANGLES_EXT 61
#define SBGC_CMD_READ_PARAMS_EXT2 62
#define SBGC_CMD_WRITE_PARAMS_EXT2 63
#define SBGC_CMD_GET_ADJ_VARS_VAL 64


#define SBGC_CMD_DEBUG_VARS_INFO_3 253
#define SBGC_CMD_DEBUG_VARS_3  254
#define SBGC_CMD_ERROR  255

// Error codes in command response
#define SBGC_ERR_CMD_SIZE 1
#define SBGC_ERR_WRONG_PARAMS 2
#define SBGC_ERR_GET_DEVICE_ID  3
#define SBGC_ERR_CRYPTO		4
#define SBGC_ERR_CALIBRATE_BAT 5
#define SBGC_ERR_UNKNOWN_COMMAND 6


// System error flags that controller may set
#define SBGC_SYS_ERR_NO_SENSOR (1<<0)
#define SBGC_SYS_ERR_CALIB_ACC (1<<1)
#define SBGC_SYS_ERR_SET_POWER (1<<2)
#define SBGC_SYS_ERR_CALIB_POLES (1<<3)
#define SBGC_SYS_ERR_PROTECTION (1<<4)
#define SBGC_SYS_ERR_SERIAL (1<<5)
#define SBGC_SYS_ERR_BAT_LOW (1<<6)
#define SBGC_SYS_ERR_BAT_CRITICAL (1<<7)
#define SBGC_SYS_ERR_GUI_VERSION (1<<8)
#define SBGC_SYS_ERR_MISS_STEPS (1<<9)
#define SBGC_SYS_ERR_SYSTEM (1<<10)
#define SBGC_SYS_ERR_EMERGENCY_STOP (1<<11)


// Trigger pins
#define SBGC_PIN_AUX1 16
#define SBGC_PIN_AUX2 17
#define SBGC_PIN_AUX3 18
#define SBGC_PIN_BUZZER 32
#define CMD_PIN_SSAT_POWER 33  // pin that control Spektrum Satellite 3.3V power line (low state enables line)

// Value passed in CMD_SERVO_OUT to disable servo output
#define SBGC_SERVO_OUT_DISABLED -1



// Control modes
#define SBGC_CONTROL_MODE_NO          0
#define SBGC_CONTROL_MODE_SPEED       1
#define SBGC_CONTROL_MODE_ANGLE       2
#define SBGC_CONTROL_MODE_SPEED_ANGLE 3
#define SBGC_CONTROL_MODE_RC          4

// Control modes
#define SBGC_CONTROL_MODE_NO          0
#define SBGC_CONTROL_MODE_SPEED       1
#define SBGC_CONTROL_MODE_ANGLE       2
#define SBGC_CONTROL_MODE_SPEED_ANGLE 3
#define SBGC_CONTROL_MODE_RC          4
#define SBGC_CONTROL_MODE_ANGLE_REL_FRAME 5



// RC channels used in the SBGC controller
#define SBGC_RC_NUM_CHANNELS 6 // ROLL, PITCH, YAW, CMD, EXT_ROLL, EXT_PITCH

// Hardware RC inputs, as labeled on the board
#define SBGC_RC_INPUT_NO 0
#define SBGC_RC_INPUT_ROLL 1
#define SBGC_RC_INPUT_PITCH 2
#define SBGC_RC_INPUT_EXT_ROLL 3
#define SBGC_RC_INPUT_EXT_PITCH 4
#define SBGC_RC_INPUT_YAW 5 // not connected in 1.0 board

// Analog inputs (board v.3.0)
#define SBGC_RC_INPUT_ADC1 33
#define SBGC_RC_INPUT_ADC2 34
#define SBGC_RC_INPUT_ADC3 35



// Bit indicates input is in analog mode
#define SBGC_RC_INPUT_ANALOG_BIT (1<<5) // 32
// Bit indicates input is a virtual channel
#define SBGC_RC_INPUT_VIRT_BIT (1<<6) // 64
// Bit indicates input is a API virtual channel
#define SBGC_RC_INPUT_API_VIRT_BIT (1<<7) // 128

// Mask to separate input channel number from input mode
#define SBGC_RC_INPUT_CH_MASK ((1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4))
#define SBGC_RC_INPUT_MODE_MASK ((1<<5) | (1<<6) | (1<<7))


// Number of virtual channels for RC serial input (s-bus, spektrum, Sum-PPM)
#define SBGC_VIRT_NUM_CHANNELS 32
// Number of virtual channels for API serial input
#define SBGC_API_VIRT_NUM_CHANNELS 32


// Normal range of RC signal. Real signal may go outside this range
#define SBGC_RC_MIN_VAL -500
#define SBGC_RC_MAX_VAL 500
// Value to encode 'RC no signal'
#define SBGC_RC_UNDEF -10000


typedef enum {
	PARSER_NO_ERROR=0,
	PARSER_ERROR_PROTOCOL=1,
	PARSER_ERROR_WRONG_CMD_SIZE=2,
	PARSER_ERROR_BUFFER_IS_FULL=3,
	PARSER_ERROR_WRONG_DATA_SIZE=4,
} SBGC_parser_errors;


//**********************************************************************************


#pragma pack(1)

// CMD_CONTROL
typedef struct {
  uint8_t mode;
  int16_t speedROLL;
  int16_t angleROLL;
  int16_t speedPITCH;
  int16_t anglePITCH;
  int16_t speedYAW;
  int16_t angleYAW;
} SBGC_cmd_control_t;

typedef struct {
  uint8_t mode[3];
  struct {
  	int16_t speed;
  	int16_t angle;
  } data[3];
} SBGC_cmd_control_ext_t;

// CMD_REALTIME_DATA_3, CMD_REALTIME_DATA_4
typedef struct {
	struct {
		int16_t acc_data;
		int16_t gyro_data;
	} sensor_data[3];  // ACC and Gyro sensor data (with calibration) for current IMU (see cur_imu field)
	int16_t serial_error_cnt; // counter for communication errors
	int16_t system_error; // system error flags, defined in SBGC_SYS_ERR_XX
	uint8_t reserved1[4];
	int16_t rc_raw_data[SBGC_RC_NUM_CHANNELS]; // RC signal in 1000..2000 range for ROLL, PITCH, YAW, CMD, EXT_ROLL, EXT_PITCH channels
	int16_t imu_angle[3]; // ROLL, PITCH, YAW Euler angles of a camera, 16384/360 degrees
	int16_t frame_imu_angle[3]; // ROLL, PITCH, YAW Euler angles of a frame, if known
	int16_t target_angle[3]; // ROLL, PITCH, YAW target angle
	uint16_t cycle_time_us; // cycle time in us. Normally should be 800us
	uint16_t i2c_error_count; // I2C errors counter
	uint8_t reserved2;
	uint16_t battery_voltage; // units 0.01 V
	uint8_t state_flags1; // bit0: motor ON/OFF state;  bits1..7: reserved
	uint8_t cur_imu; // actually selecteted IMU for monitoring. 1: main IMU, 2: frame IMU
	uint8_t cur_profile; // active profile number starting from 0
	uint8_t motor_power[3]; // actual motor power for ROLL, PITCH, YAW axis, 0..255

	// Fields below are filled only for CMD_REALTIME_DATA_4 command
	int16_t rotor_angle[3]; // relative angle of each motor, 16384/360 degrees
	uint8_t reserved3;
	int16_t balance_error[3]; // error in balance. Ranges from -512 to 512,  0 means perfect balance.
	uint16_t current; // Current that gimbal takes, in mA.
	int16_t magnetometer_data[3]; // magnetometer sensor data (with calibration)
	int8_t  imu_temp_celcius;  // temperature measured by the main IMU sensor, in Celsius
	int8_t  frame_imu_temp_celcius;  // temperature measured by the frame IMU sensor, in Celsius
	uint8_t reserved4[38];
} SBGC_cmd_realtime_data_t;


// tupes spec
typedef struct {
	uint8_t start;
	uint8_t id;
	uint8_t size;
	uint8_t hcs;
	union
	{
			SBGC_cmd_control_t  		cmd_control;
			SBGC_cmd_control_ext_t		cmd_control_ext;
			SBGC_cmd_realtime_data_t    rtdata;
		    uint8_t payloadData[SBGC_CMD_DATA_SIZE];
	};

	uint8_t CRCsumm[2];
} SerialCommand_t;

#pragma pop(0)

#endif //__SBGC__
