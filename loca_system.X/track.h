#ifndef TRACK_H
#define TRACK_H

/**
 *
 *                      y ^
 *                        |
 * _______|_______________|____>__________|_________
 *        |               |    x          |
 *        |<------------->|<------------->|
 *        |       e       |       e       |
 *        C1              T               C2
 *
 */

typedef struct Point Point;
struct Point {
  double x;
  double y;
  double z;
};

/**
 * Track()
 *
 * @brief: compute the point where the Tracker device is situated from ellapsed
 *         times.
 *
 * @param: T1 - The time ellapsed between the moment the transmitter T sent
 *              the signal and the moment the right sensor C1 received the
 *              signal back. Given in 10^(-6) second. Must be > 0.
 * @param: T2 - The time ellapsed between the moment the transmitter T sent
 *              the signal and the moment the left  sensor C2 received the
 *              signal back. Given in 10^(-6) second. Must be > 0.
 * @param: p  - The point to fill with (x, y) answer. At the end of the
 *              function, p.x and p.y contain the coordinate of the tracking
 *              device. In case of error, p will be initialized to (0, 0, 0).
 *
 * @return:  0 if no error occurs
 *          -1 if wrong input argument. Either p is null or T1 <= 0 or T2 <= 0.
 *          -2 if the computation leads to p->y < 0 which is not physically possible.
 *          -3 and -4 is a computation error (results are not real).
 */
int track (const double T1, const double T2, Point* p);

#endif

