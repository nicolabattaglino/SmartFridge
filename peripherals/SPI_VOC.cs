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
    public class SPI_VOC : ISPIPeripheral
    {

        private byte[] mem;
	    private static int i;
        public SPI_VOC()
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
            // VOC returned as value between 0 and 100 
	    mem = new Byte[] {15,26,35,48,80,97,75,65,50,32,12,6,2}; 
        
        }

        public byte Transmit(byte data)
        {
	    int dim = mem.Length;
	    byte returnData = mem[i];
        //byte returnData = data;
	    i = (i+1) % dim;
	    this.Log(LogLevel.Info, "Transmit VOC data\n");
	    return returnData;
        }

        private readonly Queue<byte> outputBuffer;
    }
}
