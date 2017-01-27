#ifndef __TIMER_H
#define __TIMER_H

// start target timeout, use global 't_timeout_ticks' as time when TIFR.OCF1A will be set to indicate timeout
void t_timeout_start(void);

void t_timeout_stop(void);

void t_timeout_after_ms(int t_ms);

// returns 1 if t_timeout has expired
int t_timeout_check(void);

void t_timeout_clear(void);

#endif  // __TIMER_H
