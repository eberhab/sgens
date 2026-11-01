/* The adapter owns visible presentation, including its logical raster clock.
 * No hidden backend raster query or physical monitor wait belongs on this path.
 * 60 Hz, 480 active lines, final 10% of each cycle blank. Not physical VSync.
 * All four APIs share this clock; waits yield the CPU until the next boundary.
 * XP-compatible QPC, with a wrap-safe GetTickCount fallback. No timer-resolution
 * changes, busy waiting, backend-name checks or C runtime dependencies.
 */
DLLIMPORT BOOL WINAPI QueryPerformanceCounter(long long *);
DLLIMPORT BOOL WINAPI QueryPerformanceFrequency(long long *);
DLLIMPORT void WINAPI Sleep(DWORD);
#define LOGICAL_HZ 60ul
#define LOGICAL_BLANK 900ul
#define DDERR_VERTICALBLANKINPROGRESS ((HRESULT)0x88760219ul)
static unsigned logical_shift,logical_initialized;
static DWORD logical_frequency,logical_last_ms,logical_fallback_phase;
/* Only the divisor is bounded. Long division handles the entire counter without
 * overflow, loss of uptime precision or compiler-specific 64-bit CRT helpers. */
static DWORD logical_remainder(unsigned long long value,DWORD divisor)
{
    DWORD remainder=0;unsigned i;
    for(i=0;i<64;i++){
        remainder=(remainder<<1)|(DWORD)(value>>63);value<<=1;
        if(remainder>=divisor)remainder-=divisor;
    }
    return remainder;
}
static DWORD logical_phase(void)
{
    long long ticks,frequency;
    if(!logical_initialized){
        logical_initialized=1;logical_last_ms=GetTickCount();
        if(QueryPerformanceFrequency(&frequency) && frequency>=6000){
            while(frequency>1000000){frequency>>=1;++logical_shift;}
            logical_frequency=(DWORD)frequency;
        }
    }
    if(logical_frequency && QueryPerformanceCounter(&ticks)){
        DWORD remainder=logical_remainder(((unsigned long long)ticks)>>logical_shift,logical_frequency);
        return ((remainder*LOGICAL_HZ)%logical_frequency)*1000/logical_frequency;
    }
    DWORD now=GetTickCount(),elapsed=now-logical_last_ms;
    logical_last_ms=now;
    logical_fallback_phase=(logical_fallback_phase+(elapsed%1000)*LOGICAL_HZ)%1000;
    return logical_fallback_phase;
}
static DWORD logical_wait_ms(DWORD phase,DWORD flags)
{
    DWORD boundary=(flags==1 && phase<LOGICAL_BLANK)?LOGICAL_BLANK:(flags==1?1000+LOGICAL_BLANK:1000);
    return (boundary-phase+LOGICAL_HZ-1)/LOGICAL_HZ;
}
