//
// Copyright (c) 2010-2018 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//
using System;
using System.Collections.Generic;
using System.Linq;
using Antmicro.Renode.Exceptions;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.SPI;
using Antmicro.Renode.Peripherals.Sensor;
using Antmicro.Renode.Utilities;

using Antmicro.Renode.Peripherals.I2C;

namespace Antmicro.Renode.Peripherals.Sensor
{
    public class SPI_temperature : ISPIPeripheral
    {

        private byte[] mem;
	    private static int i;
        public SPI_temperature()
        {
            outputBuffer = new Queue<byte>();
            Reset();
        }

        public void FinishTransmission()
        {
        }

        public void Reset()
        {
	    i = 0;
	    // temperatures returned as value between 0 and 255 corresponding to the range [-40, 110] (?) (150/255=0.588^C)
	    mem = new Byte[] {76,78,76,82,84,86,74,76,54,52,50,60,70}; 
        }

        public byte Transmit(byte data)
        {
	    int dim = mem.Length;
	    byte returnData = mem[i];
        //byte returnData = data;
	    i = (i+1) % dim;
	    this.Log(LogLevel.Info, "Transmit temperature\n");
	    return returnData;
        }

        private readonly Queue<byte> outputBuffer;
    }
}
