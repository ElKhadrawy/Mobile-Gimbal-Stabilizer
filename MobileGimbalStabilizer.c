        #define IMU_WRITE 0b11010000  // Write Address      AD0=0
         #define IMU_READ 0b11010001   // Read Address       AD0=0
         unsigned char word [16];
         unsigned char title [16];
         unsigned char TempH;
         unsigned char TempL;
void IMU_Init()
{
     TRISC = 0x00;           //TX o/p
     UART1_Init(9615);       //Baud rate
     Delay_ms(500);
     I2C1_Init(400000);      //Fast Frequency(400K), we could use 100K also
     I2C1_Start();
     I2C1_Wr(IMU_WRITE);     //Write PWR_MGMT_1 Address
     I2C1_Wr(0x6B);          //Should I wait for acknowledgement  ?             K.opinion: ac:Repeated_start  "Each byte you write to the slave device should be answered with an ACK if the operation was successful."  Should we implement a method that returns a boolean to check this?
                             //Reset, Sleep, Cycle, N/A, TempSensor Disable, Clk SC: PLL X Gyro
     I2C1_Wr(0b00001001);    //K., set the reset bit.
     I2C1_Stop();
     Delay_ms(70);            //allow time for stop to be sent *MIT*
     I2C1_Start();
     I2C1_Wr(IMU_WRITE);     //Write GYRO_CONFIG Address
     I2C1_Wr(0x1B);          //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(0x00);          // Self test off[7:5], Full Scale Range: +-250 Deg/Sec
     I2C1_Stop();
     Delay_ms(70);            //allow time for stop to be sent *MIT*
     I2C1_Start();
     I2C1_Wr(IMU_WRITE);     //Write ACCEL_CONFIG Address
     I2C1_Wr(0x1C);          //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(0x00);          // Self test off[7:5], Full Scale Range: +-2g
     I2C1_Stop();
     Delay_ms(70);            //allow time for stop to be sent *MIT*
}
void IMU_Write(unsigned char regAddress, unsigned char regData )
{
     I2C1_Start();
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(regAddress);    //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(regData);
     I2C1_Stop();
     Delay_ms(1);            //allow time for stop to be sent *MIT*
}
signed int IMU_Read(unsigned char regAddress)  // Used for two consecutive reads
{
     signed int outReg;
     I2C1_Start();                           //Write ACC X Address
     I2C1_Wr(IMU_WRITE);     //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(regAddress);                          //Read regAddress Data
     I2C1_Repeated_Start();           // The Data sheet states that we should "start", we are not sure if we should "Restart".  K.opinion: ac:Repeated_start  // as mentioned in the answer that we need to start again without a stop and he called it a "restart" which is Repeated_Start method in our UART library.
     I2C1_Wr(IMU_READ);      // K., Should we Write the number of registers that we're targeting?                       K.opinion cont.: ac:Repeated_Start_2    ac:Repeated_Start_3       // Supports S.opinion.
     TempH=I2C1_Rd(1);
     while (!I2C1_Is_Idle()) asm nop;  // Wait for the read cycle to finish
     TempL=I2C1_Rd(0);
     while (!I2C1_Is_Idle()) asm nop;   // Wait for the read cycle to finish
     outReg=TempH<<8|TempL; // Concatenate the High and Low Registers
     I2C1_Stop();
     Delay_ms(1);            //allow time for stop to be sent *MIT*
     return outReg;
}
void print(signed int number,unsigned char title [16])
{
     IntToHex(number, word);
     UART1_Write_Text(title);
     UART1_Write_Text(word);
     UART1_Write_Text("                                                                     ");
     Delay_ms(100);
}
signed int getAccAngle(signed int ACC_sub,signed int ACC)
{
     return acos(ACC_sub/ACC);
}
void main() {
     signed int ACC;
     signed int ACC_X;
     signed int Angle_ACC_X;
     signed int ACC_Y;
     signed int Angle_ACC_Y;
     signed int ACC_Z;
     signed int Angle_ACC_Z;
     signed int GYRO_X;
     signed int GYRO_Y;
     signed int GYRO_Z;
     IMU_Init();
     IMU_Write(0x1A, 0x03);         //CONFIG Register, Digital Low Pass Filter: 3 *MIT*
    while(1)
     {
     ACC_X=IMU_Read(0x3B);
     print(ACC_X,"X.acce:");
     GYRO_X=IMU_Read(0x43);
     print(GYRO_X,"X.Gyro:");
     ACC_Y=IMU_Read(0x3D);
     print(ACC_Y,"Y.acce:");
     ACC=SQRT(ACC_X^2+ACC_Y^2 +ACC_Z^2);
     Angle_ACC_X=getAccAngle(ACC_X,ACC);
     print(Angle_ACC_X,"X.A.An:");
     Angle_ACC_Y=getAccAngle(ACC_Y,ACC);
     print(Angle_ACC_Y,"Y.A.An:");
     Angle_ACC_Z=getAccAngle(ACC_Z,ACC);
     print(Angle_ACC_Z,"Z.A.An:");
     GYRO_Y=IMU_Read(0x45);
     print(GYRO_Y,"Y.Gyro:");
     ACC_Z=IMU_Read(0x3F);
     print(ACC_Z,"Z.acce:");
     GYRO_Z=IMU_Read(0x47);
     print(GYRO_Z,"Z.Gyro:");
     }
}
