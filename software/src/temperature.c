/* temperature-bricklet
 * Copyright (C) 2010-2013 Olaf Lüke <olaf@tinkerforge.com>
 *
 * temperature.c: Implementation of Temperature Bricklet messages
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "temperature.h"

#include "brickletlib/bricklet_entry.h"
#include "brickletlib/bricklet_simple.h"
#include "bricklib/bricklet/bricklet_communication.h"
#include "config.h"

#define I2C_EEPROM_ADDRESS_HIGH 84

#define I2C_ADDRESS_HIGH 73 // 0b1001001
#define I2C_ADDRESS_LOW 72 // 0b1001000
#define I2C_INTERNAL_ADDRESS_TEMPERATURE 0x0
#define I2C_INTERNAL_ADDRESS_BYTES 1
#define I2C_DATA_LENGTH 2

// Temperature is defined by a multiplier of 0.0625, thus means we get
// 100th degree if we do value*100/16
#define TEMP_SCALE_MUL 100
#define TEMP_SCALE_DIV 16

#define SIMPLE_UNIT_TEMPERATURE 0

#define TWITIMEOUTMAX 50000


const SimpleMessageProperty smp[] = {
	{SIMPLE_UNIT_TEMPERATURE, SIMPLE_TRANSFER_VALUE, SIMPLE_DIRECTION_GET}, // TYPE_GET_TEMPERATURE
	{SIMPLE_UNIT_TEMPERATURE, SIMPLE_TRANSFER_PERIOD, SIMPLE_DIRECTION_SET}, // TYPE_SET_TEMPERATURE_CALLBACK_PERIOD
	{SIMPLE_UNIT_TEMPERATURE, SIMPLE_TRANSFER_PERIOD, SIMPLE_DIRECTION_GET}, // TYPE_GET_TEMPERATURE_CALLBACK_PERIOD
	{SIMPLE_UNIT_TEMPERATURE, SIMPLE_TRANSFER_THRESHOLD, SIMPLE_DIRECTION_SET}, // TYPE_SET_TEMPERATURE_CALLBACK_THRESHOLD
	{SIMPLE_UNIT_TEMPERATURE, SIMPLE_TRANSFER_THRESHOLD, SIMPLE_DIRECTION_GET}, // TYPE_GET_TEMPERATURE_CALLBACK_THRESHOLD
	{0, SIMPLE_TRANSFER_DEBOUNCE, SIMPLE_DIRECTION_SET}, // TYPE_SET_DEBOUNCE_PERIOD
	{0, SIMPLE_TRANSFER_DEBOUNCE, SIMPLE_DIRECTION_GET}, // TYPE_GET_DEBOUNCE_PERIOD
};

const SimpleUnitProperty sup[] = {
	{get_temperature, SIMPLE_SIGNEDNESS_INT, FID_TEMPERATURE, FID_TEMPERATURE_REACHED, SIMPLE_UNIT_TEMPERATURE} // temperature
};

const uint8_t smp_length = sizeof(smp);

void invocation(const ComType com, const uint8_t *data) {
	switch(((MessageHeader*)data)->fid) {

		case FID_GET_TEMPERATURE:
		case FID_SET_TEMPERATURE_CALLBACK_PERIOD:
		case FID_GET_TEMPERATURE_CALLBACK_PERIOD:
		case FID_SET_TEMPERATURE_CALLBACK_THRESHOLD:
		case FID_GET_TEMPERATURE_CALLBACK_THRESHOLD:
		case FID_SET_DEBOUNCE_PERIOD:
		case FID_GET_DEBOUNCE_PERIOD: {
			simple_invocation(com, data);
			break;
		}

		case FID_SET_I2C_MODE: {
			set_i2c_mode(com, (SetI2CMode*)data);
			break;
		}

		case FID_GET_I2C_MODE: {
			get_i2c_mode(com, (GetI2CMode*)data);
			break;
		}

		default: {
			BA->com_return_error(data, sizeof(MessageHeader), MESSAGE_ERROR_CODE_NOT_SUPPORTED, com);
			break;
		}
	}
}

void constructor(void) {
	_Static_assert(sizeof(BrickContext) <= BRICKLET_CONTEXT_MAX_SIZE, "BrickContext too big");

	BC->i2c_mode = I2C_MODE_FAST;
	simple_constructor();
}

void destructor(void) {
	simple_destructor();
}

void tick(const uint8_t tick_type) {
	simple_tick(tick_type);
}

int32_t get_temperature(const int32_t value) {
	int32_t new_value = value;
    if((BC->tick % 250) == 0) {
    	if(BA->mutex_take(*BA->mutex_twi_bricklet, 10)) {
    		new_value = temperature_read();
    		BA->mutex_give(*BA->mutex_twi_bricklet);
    	}
    }
    return new_value;
}

int16_t two_complement_12_to_16(const int16_t value) {
	if(value & (1 << 11)) {
		return value | 0xF000;
	}

	return value;
}

int16_t temperature_read(void) {
	const uint8_t port = BS->port - 'a';

	uint8_t value[2];

    uint32_t timeout;

	if(BC->i2c_mode == I2C_MODE_SLOW) {
		// Switch to 100khz
		BA->twid->pTwi->TWI_CWGR = 0;
		BA->twid->pTwi->TWI_CWGR = (1 << 16) | (158<< 8) | 158;
	}

	BA->bricklet_select(port);

	BA->TWID_Read(BA->twid,
	              BS->address == I2C_EEPROM_ADDRESS_HIGH ? I2C_ADDRESS_HIGH : I2C_ADDRESS_LOW,
	              0, 0,
				  value, 2,
				  NULL);

	BA->bricklet_deselect(port);

	// Switch back to 400khz
	BA->twid->pTwi->TWI_CWGR = 0;
	BA->twid->pTwi->TWI_CWGR = (76 << 8) | 76;

	return two_complement_12_to_16((value[1] | (value[0] << 8)) >> 4)*TEMP_SCALE_MUL/TEMP_SCALE_DIV;
}

void get_i2c_mode(const ComType com, const GetI2CMode *data) {
	GetI2CModeReturn gi2cmr;

	gi2cmr.header         = data->header;
	gi2cmr.header.length  = sizeof(GetI2CModeReturn);
	gi2cmr.mode           = BC->i2c_mode;

	BA->send_blocking_with_timeout(&gi2cmr, sizeof(GetI2CModeReturn), com);
}

void set_i2c_mode(const ComType com, const SetI2CMode *data) {
	if(data->mode > I2C_MODE_SLOW) {
		BA->com_return_error(data, sizeof(MessageHeader), MESSAGE_ERROR_CODE_INVALID_PARAMETER, com);
		return;
	}

	BC->i2c_mode = data->mode;
	BA->com_return_setter(com, data);
}
