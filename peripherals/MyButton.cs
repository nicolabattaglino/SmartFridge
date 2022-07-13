using Antmicro.Renode.Core;
using System;
using Antmicro.Renode.Utilities;
using Antmicro.Renode.Logging;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class MyButton : IPeripheral, IGPIOSender
    {
        public MyButton(bool invert = false)
        {
            Inverted = invert;
            IRQ = new GPIO();

            Reset();
        }

        public void Reset()
        {
            // We call Press here to refresh states after reset.
            Press();
            Release();
            Press();
        }

        public void PressAndRelease()
        {
            Press();
            Release();
        }

        public void Press()
        {
            IRQ.Set(!Inverted);
            Pressed = true;
            OnStateChange(true);
        }

        public void Release()
        {
            IRQ.Set(Inverted);
            Pressed = false;
            OnStateChange(false);
        }

        public void Toggle()
        {
            
            if(Pressed)
            {
                this.Log(LogLevel.Info, " state changed to release, fridge is open");
                Release();
            }
            else
            {
                this.Log(LogLevel.Info, " state changed to press, fridge is closed");
                Press();
            }
        }

        private void OnStateChange(bool pressed)
        {
            var sc = StateChanged;
            if (sc != null)
            {
                sc(pressed);
            }
        }

        public GPIO IRQ { get; private set; }

        public event Action<bool> StateChanged;

        public bool Pressed { get; private set; }

        public bool Inverted { get; private set; }
    }
}
