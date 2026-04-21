/*
 ratio.h file - function defining ratio computation and reporting for compression results
 */

#ifndef RATIO_H
#define RATIO_H

#include <stddef.h>

typedef struct {
    double ratio;           /* 0.0 – 1.0, fraction of space saved     */
    double percent_saved;   /* ratio * 100                             */
    double original_kb;
    double compressed_kb;
} RatioReport;

/* ratioo computation*/
RatioReport ratio_compute(size_t original_bytes, size_t compressed_bytes);

/* Print ratio report to stdout (used outside GUI) */
void ratio_print(const RatioReport *r);

/* Write ratio report to a file in apopned mode */
void ratio_log_to_file(const RatioReport *r, const char *path);

#endif 
