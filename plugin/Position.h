
#ifndef XPLANE_NANOMSG_POSITION_H
#define XPLANE_NANOMSG_POSITION_H


#include "DataRefManager.h"

#define FEET_PER_METER 3.28084

struct PosRefs {
  DataRefInfo *x, *y, *z;
  DataRefInfo *lat, *lon;
  DataRefInfo *lat_ref, *lon_ref;
  DataRefInfo *elev, *y_agl;  // alt is calculated from elv
  DataRefInfo *heading, *pitch, *roll;
  DataRefInfo *q;  // q is not exposed
  DataRefInfo *true_heading, *true_pitch, *true_roll;  // TODO: experimental
  DataRefInfo *mag_heading;
};

// The aircraft position
// See https://developer.x-plane.com/article/movingtheplane/
class Position {
private:
  DataRefManager *mgr;
  PosRefs refs;

public:
  explicit Position(DataRefManager *mgr);

  void update();
  void setPositionFeet(double newLat, double newLon, double newAlt);
  void setPositionMetric(double newLat, double newLon, double newElev);
  void toProtobufData(xplane::Position * p) const;

  double x, y, z;             // meters
  double lat, lon;            // degrees
  float lat_ref, lon_ref;     // degrees of 0 coordinates
  double elev;                // meters above MSL
  double alt;                 // feet above MSL
  float y_agl;                // meters
  // TODO: check psi vs true_psi, etc
  float pitch, roll, heading; // degrees
  float true_pitch, true_roll, true_heading; // degrees
  float mag_heading; // degrees
};


#endif //XPLANE_NANOMSG_POSITION_H
