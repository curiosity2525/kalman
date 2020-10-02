#include "mbed.h"
Serial uart_usb(USBTX, USBRX); //UART(over USB)
I2C i2c(PB_9, PB_8); //Gyro + Accelerometer(SDA, SCLK)

void i2c_mem_write(int device_address, int mem_address, int mem_data)
{
    int device_address_temp = device_address << 1;
    device_address_temp = device_address_temp & 0xfe;
    
    i2c.start();
    i2c.write(device_address_temp);
    i2c.write(mem_address);
    i2c.write(mem_data);
    i2c.stop();
    return;
}

//i2c read function
int i2c_mem_read(int device_address, int mem_address)
{
    int device_address_temp = device_address << 1;
    int device_address_temp_w = device_address_temp & 0xfe;
    int device_address_temp_r = device_address_temp | 0x01;
    
    i2c.start();
    i2c.write(device_address_temp_w);
    i2c.write(mem_address);
    i2c.start();
    i2c.write(device_address_temp_r);
    int data = i2c.read(0);
    i2c.stop();
    return data;
}

//Gyro
float get_gyro_x_data()
{
    //read RATE_X_LSB registor
    int x_temp_L = i2c_mem_read(0x69, 0x02);
    //read RATE_X_MSB registor
    int x_temp_H = i2c_mem_read(0x69, 0x03);
    
    //calcu;ate X angular ratio
    int x_data = x_temp_L + 256*x_temp_H;
    if(x_data > 32767)
    {
        x_data = -1*(65536 - x_data);
    }
    
    x_data = -1*x_data;
    //+1000(deg/sec)/2^15 = 0.0305176
    return float(x_data)*0.0305176f;
}

int main()
{
    //UART initialization
    uart_usb.baud(115200);
    uart_usb.format(8, Serial::None, 1);
    
    //I2C initialization
    i2c.frequency(400000); //400kHz
    
    //initialize Gyro registor 0x0F(range)
    //Full scale = +/- 1000deg/s
    i2c_mem_write(0x69, 0x0f, 0x01);
    
    //initialize Gyro register 0x10(band width)
    //Data rate = 1000Hz, Filter bandwidth = 116Hz
    i2c_mem_write(0x69, 0x10, 0x02);
    
    //loop
    float temp;
    while(1)
    {
        temp = get_gyro_x_data();
        uart_usb.printf("gyro x = %f(deg/sec)\r\n", temp);
        wait(0.1);
    }
}
    