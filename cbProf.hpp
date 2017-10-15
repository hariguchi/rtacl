#ifndef __CBPROF_HPP__
#define __CBPROF_HPP__

/*
 * Copyright (c) 2017 Yoichi Hariguchi
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 */

#include <time.h>
#include <chrono>
#include "local_types.h"


namespace cbProf {

using bfmt = boost::format;
using nsec = std::chrono::nanoseconds;
using usec = std::chrono::microseconds;
using msec = std::chrono::milliseconds;
using hrclock = std::chrono::high_resolution_clock;
using timePoint = std::chrono::time_point<hrclock>;

/**
 *  @name  prof
 *  @brief Measures the performance of a code block
 *
 *   Example
 *
 *   cbProf::prof prof("Example: ", true);
 *
 *   for ( i = 0; i < 1000; ++i ) {
 *       prof.begin();
 *       functionToBeProfiled();
 *       prof.end();
 *   }
 *   prof.makeHist();
 *   printf("%s\n", prof.getCstr());
 */
class prof {
private:
    u32      nCalls;            // the number of calls
    bool     running;           // true: running, false: not nunning
    timePoint start;
    nsec min;
    nsec max;
    nsec sum;
    /*
     * histogram:
     *  hist[0..9]:   0ns   - 1000ns (100ns granularity)
     *  hist[10..18]: 1us   - 10us   (1us granularity)
     *  hist[19..27]: 10us  - 100us  (10us granularity)
     *  hist[28..36]: 100us - 1000us (100us granularity)
     *  hist[37]: >1ms
     */
    u32  hist[38];
    std::string msg;
    std::string banner;
public:
    prof (const char* s) {
        init();
        setBanner(s);
    };
    prof (bool r) { prof("", r); };
    prof (const char* s="", bool r=true) {
        init();
        setBanner(s);
        running = r;
    };
    void init();
    void setBanner (const std::string& s) {
        banner = s;
    }
    void run () { running = true; };
    void stop () { running = false; };
    void begin();
    void end();
    void makeHist();
    const std::string& str() const { return msg; };
    const char* getCstr() const { return msg.c_str(); };
private:
    void makeHistEnt(int begin, int end, int tm, int d,
                     const char* fmt1, const char* fmt2);
};

/**
 * @name  prof::init
 * @brief Public function
 *        Initializes internal data structures.
 */
inline void
prof::init ()
{
    memset(hist, 0, sizeof(hist));
    min = nsec::max();
    max = nsec::min();
    sum = nsec::zero();
    nCalls = 0;
    running = false;
}

/**
 * @name  prof::begin
 * @brief Public function
 *        Starts the stopwatch
 */
inline void
prof::begin ()
{
    if ( !running ) {
        return;
    }
    start = hrclock::now();
}

/**
 * @name  prof::end
 * @brief Public function
 *        Stops the stopwatch, then updates the duration statistics
 */
inline void
prof::end ()
{
    if (!running) {
        return;
    }
    nsec delta = hrclock::now() - start;

    if (delta >= nsec(1000*1000)) {
        /*
         * >= 1ms. Omit from profiling, but keep the record
         */
        ++hist[37];
        return;
    }
    /*
     * Update the histogram
     */
    ++nCalls;                   // number of calls
    sum += delta;
    u32 i;
    if (delta < nsec(1000)) {
        i = delta.count() / 100;
    } else if (delta < nsec(10*1000)) {
        i = 9 + (delta.count()/1000);
    } else if (delta < nsec(100*1000)) {
        i = 18 + (delta.count()/10000);
    } else if (delta < nsec(1000*1000)) {
        i = 27 + (delta.count()/100000);
    }
    ++hist[i];

    if (delta > max) {
        max = delta;
    }
    if (delta < min) {
        min = delta;
    }
}

/**
 * @name  prof::makeHistEnt
 * @brief Private function
 *        Makes a histogram entry
 *
 * @param[in] begin First index of hist[] to calculate the sum
 * @param[in] end   Last index of hist[] to calculate the sum
 * @param[in] tm    Start time
 * @param[in] d     Duration
 * @param[in] fmt1  Format string to boost::format() if the
 *                  sum of hist[begin..end] is <= 1%
 * @param[in] fmt2  Format string to boost::format() if the
 *                  sum of hist[begin..end] is > 1%
 */
inline void
prof::makeHistEnt (int begin, int end, int tm, int d,
                       const char* fmt1, const char* fmt2)
{
    double r;
    double nc = (double)this->nCalls;
    u32    sum;
    int    i;

    sum = 0;
    for ( i = begin; i < end; ++i ) {
        sum += hist[i];
    }
    /*
     * If the sum of hist[begin..end] is < 1%,
     * output only the summary. Otherwise output the details.
     */
    r = ((double)sum) / nc;
    if (r <= 0.01) {
        msg += banner;
        msg += (bfmt(fmt1) % (r * 100.0) % sum).str();
    } else {
        for (i = begin; i < end; ++i, tm += d) {
            msg += banner;
            msg += (bfmt(fmt2)
                    % tm
                    % (tm + d)
                    % ((((double)hist[i]) * 100.0) / nc)
                    % hist[i]).str();
        }
    }
}

/**
 * @name  prof::makeHist
 * @brief Public function
 *        Makes a histogram
 */
inline void
prof::makeHist ()
{
    double usecMax = ((double)max.count())/1000.0;
    double usecSum = ((double)sum.count())/1000.0;

    msg = (bfmt("%s %d calls, %.2f us, "
                "%.2f us/call, min: %ld ns, max: %ld us\n")
           % banner
           % nCalls
           % usecSum
           % (usecSum/(double)nCalls)
           % min.count()
           % usecMax).str();
    /*
     * 0ns - 1000ns
     */
    makeHistEnt(0, 10, 0, 100,
                    "     0ns - 1000ns: %5.2f%%  %d\n",
                    "  %4dns - %4dns: %5.2f%%  %d\n");
    /*
     * 1us - 10us
     */
    makeHistEnt(10, 19, 1, 1,
                "     1us - 10us:   %5.2f%%  %d\n",
                "  %4dus - %2dus:   %5.2f%%  %d\n");
    /*
     * 10 - 100us
     */
    makeHistEnt(19, 28, 10, 10,
                "    10us - 100us:  %5.2f%%  %d\n",
                "  %4dus - %3dus:  %5.2f%%  %d\n");
    /*
     * 100us - 1000us
     */
    makeHistEnt(28, 37, 100, 100,
                "   100us - 1000us: %5.2f%%  %d\n",
                "  %4dus - %4dus: %5.2f%%  %d\n");

    msg += (bfmt("%s          >1ms:      ---   %d\n")
            % banner
            % hist[37]).str();
}

} // namespace cbProf

#endif//__CBPROF_HPP__
