// Automatically generated C++ file on Mon Apr  7 13:22:36 2025
//
// To build with Digital Mars C++ Compiler:
//
//    dmc -mn -WD test_x1.cpp kernel32.lib

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
#undef IN
#undef OUT

extern "C" __declspec(dllexport) void test_x1(void **opaque, double t, union uData *data)
{
   double  IN  = data[0].d; // input
   double &OUT = data[1].d; // output

// Implement module evaluation code here:
   OUT = IN * IN;

}
