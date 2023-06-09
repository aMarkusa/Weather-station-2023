#include "bme680_task.h"


struct bme68x_dev sensor;
struct bme68x_conf sensor_conf;
struct bme68x_heatr_conf sensor_heatr_conf;
struct bme68x_data data;


int8_t perform_reading(){
	uint8_t n_fields;
	
	int8_t rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &sensor);
	uint32_t reading_duration = (uint32_t)bme68x_get_meas_dur(BME68X_FORCED_MODE, &sensor_conf, &sensor) +
                            ((uint32_t)sensor_heatr_conf.heatr_dur * 1000);
	
	if(reading_duration == 0){
		return false;
	}
	// wait for reading_duration to end
	delay_usec(reading_duration, NULL);
	
	// fetch the measured data
	rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &sensor);
	
	return rslt;
}

uint8_t read_sensor(float sensor_data[4]){	
	sensor.chip_id = 0x61;
  sensor.intf = BME68X_I2C_INTF;
  sensor.intf_ptr = NULL;
  sensor.read = &i2c_read;
  sensor.write = &i2c_write;
  sensor.amb_temp = 25; /* The ambient temperature in deg C is used for
																//defining the heater temperature */
  sensor.delay_us = &delay_usec;
	if(bme68x_init(&sensor) != 0){
		return 1;
	}

	sensor_conf.os_temp = BME68X_OS_2X;
	sensor_conf.os_hum = BME68X_OS_16X;
	sensor_conf.os_pres = BME68X_OS_1X;
	sensor_conf.odr = BME68X_ODR_NONE;
	sensor_conf.filter = BME68X_FILTER_OFF;
	int8_t rslt = bme68x_set_conf(&sensor_conf, &sensor);
	if(rslt != BME68X_OK){
		return 1;
	}

	sensor_heatr_conf.enable = BME68X_ENABLE;
	sensor_heatr_conf.heatr_temp = 300;
	sensor_heatr_conf.heatr_dur = 100;
	rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &sensor_heatr_conf, &sensor);
	if(rslt != BME68X_OK){
		return 1;
	}
	
	if(!perform_reading()){
		sensor_data[0] = data.temperature;
		sensor_data[1] = data.pressure;
		sensor_data[2] = data.humidity;
		sensor_data[3] = data.gas_resistance;
		
		return 0;
	}
	
	else{
		return 0;
	}
			
}

