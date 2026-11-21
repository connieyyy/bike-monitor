#ifndef BIKE_BUTTON_H
#define BIKE_BUTTON_H

enum class BikeDisarmAttempt { None, Accepted, Rejected };

struct BikeButtonUpdate {
    bool armed;
    BikeDisarmAttempt disarmAttempt;
};

void bikeButtonBegin();
BikeButtonUpdate bikeButtonUpdate(bool bikeMoving, bool alarmTriggered);

#endif
