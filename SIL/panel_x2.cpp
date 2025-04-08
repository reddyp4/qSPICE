// Automatically generated C++ file on Tue Apr  8 09:41:59 2025
//
// To build with Digital Mars C++ Compiler:
//
//    dmc -mn -WD panel_x2.cpp kernel32.lib

union uData
{
   bool b;
   char c;
   unsigned char uc;
   short s;
   unsigned short us;
   int i;
   unsigned int ui;
   float f;
   double d;
   long long int i64;
   unsigned long long int ui64;
   char *str;
   unsigned char *bytes;
};

// int DllMain() must exist and return 1 for a process to load the .DLL
// See https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain for more information.
int __stdcall DllMain(void *module, unsigned int reason, void *reserved) { return 1; }

// #undef pin names lest they collide with names in any header file(s) you might include.
#undef vin
#undef vout
#undef iout
#undef iin

void mppt_controller(double vin,double in,double *vout,double *iout) {
   // observe delta Pout and perturb with vin_targ step
   // runs every 100 ms
   //*vout = vin*10;

   int32_t mppt_pout = pout_tot;

   if (OutState != (OUTRAMP60 || OUTOFF) && BalState == BALDIS) { // turn on balancing if not actively ramping, auto checks if all pins are connected
      start_BALRAMP();
   }

   if (OutState != OUTRAMP60 && (mppt_pout) < 10000) {
      //disable_BAL();
   }

   // occasionally pulseoff feature to bias string to 100%
   if (mppt_pulseoff_cnt == 1000) {
      mppt_pulseoff_num = 1200 + duty_out / 5 + (adcslow.temp_board % 10) * 10; // time for pulseoff 2min + k*duty + small random
   }
   else if (mppt_pulseoff_cnt == mppt_pulseoff_num) { // turn off
      OutState = OUTOFF;
      HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2); // disable but don't turn off interrupt
      //disable_BAL();
   }
   else if (mppt_pulseoff_cnt > (mppt_pulseoff_num + 5)) { // turn on after 500ms and ramp
      mppt_pulseoff_cnt = 0;
      mppt_10s_cnt = 0;
      HAL_TIM_Base_Stop_IT(&htim6);
      start_OUTRAMP60();
   }

   if (mppt_10s_cnt > MPPT_10S_NUM) { // keep track of max vmp in the past 10s and update vmp to bias state transitions
      mppt_10s_cnt = 0;
      vmp_hold = saturate(vmp_hold_next, VMP_MIN, VMP_MAX);
      pmp_hold = 0;
   }
   if (mppt_pout > pmp_hold) { // record vin at max pout
      pmp_hold = mppt_pout;
      vmp_hold_next = vin_tot;
   }

   if (OutState == OUT100) {
      mppt_pulseoff_cnt = 0;
      if (vin_meas < (vmp_hold - 6000) || vin_meas < VMP_MIN) { // xxx tune 6V to be slightly greater than typical MPPT step
         vin_targ = saturate(vmp_hold - 100, VMP_MIN, VMP_MAX);
         set_OUTVMP(9900);
      }
   }
   else if (OutState == OUTVMP) {
      if (duty_out >= GAIN_OUT_MAX) { // limited by load presented by inverter, move to OUT100
         OutState = OUT100;
         vmp_hold = saturate(vin_meas, VMP_MIN, VMP_MAX);
         vin_targ = saturate(vin_meas, VMP_MIN, VMP_MAX);
      }
      else if ((mppt_pout) < 10000 && vin_meas > (VIN_RUN_MIN1+ 3000)) { // low power <10W just stay at 100% duty
         OutState = OUT100;
         vin_targ = saturate(vin_targ - 100, VMP_MIN, VMP_MAX);
      }
      else if ((mppt_pout) < 1000) { //
         vin_targ = saturate(vin_targ - 100, VMP_MIN, VMP_MAX); // protect from vin_targ > vin_meas and keep in 100% duty
      }
      else {
         if (mppt_pout < mppt_pout_prev) {
            mppt_stepsign *= -1;
         }
         int32_t p_delta = mppt_pout - mppt_pout_prev;
         int32_t p_delta_norm = abs(p_delta / (mppt_pout_prev / 1000)); // 50 -> 5.00%
         int32_t vin_targ_step = saturate(p_delta_norm * VMP_PGAIN / 1000, VMP_STEP_MIN, VMP_STEP_MAX);
         vin_targ = saturate(vin_targ + mppt_stepsign * vin_targ_step, VMP_MIN, VMP_MAX);
      }
   }

   mppt_pout_prev = mppt_pout;
   mppt_cnt++;
   mppt_10s_cnt++;
   mppt_pulseoff_cnt++;
}

extern "C" __declspec(dllexport) void panel_x2(void **opaque, double t, union uData *data)
{
   double  vin  = data[0].d; // input
   double  iin  = data[1].d; // input
   double &vout = data[2].d; // output
   double &iout = data[3].d; // output

// Implement module evaluation code here:
   mppt_controller(vin,iin,&vout,&iout);
}
