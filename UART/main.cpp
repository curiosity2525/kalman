#include "mbed.h"
Serial uart_usb(USBTX, USBRX); //UART(over USB)
I2C i2c(PB_9, PB_8); //Gyro + Accelerometer(SDA, SCLK)

//====================================================================

//i2c write function
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



//Accelerometer(BMX055)
float get_acc_z_data()
{
    //read ACC_Z_LSB registor(0x06)
    int z_temp_L = i2c_mem_read(0x19, 0x06);
    z_temp_L = z_temp_L >> 4;
    z_temp_L = z_temp_L & 0x0f;
    
    //read ACC_Z_MSB registor(0x07)
    int z_temp_H = i2c_mem_read(0x19, 0x07);
    
    //calculate Z acceleration
    int z_data = z_temp_L + 16 * z_temp_H;
    if(z_data > 2047)
    {
        z_data = -1 * (4096 - z_data);
    }
    
    //1bit sign + 11bit(fullscale +/- 2G)
    return (float)z_data/1024;
}

//main
int main(){
    //UART initialization
    uart_usb.baud(115200);
    uart_usb.format(8, Serial::None, 1);
    
    //I2C initialization
    i2c.frequency(400000); //400kHz
    
    //initialize ACC register 0x0F(range)
    //Full scale = +/- 2G
    i2c_mem_write(0x19, 0x0f, 0x03);
    
    //initialize ACC register 0x10(band width)
    //Filter bandwidth = 1000Hz
    i2c_mem_write(0x19, 0x10, 0x0f);
    
    //loop
    float temp;
    while(1)
    {
        temp = get_acc_z_data();
        uart_usb.printf("acc z = %f(G)\r\n", temp);
        wait(0.1);
    }
}


