
#ifndef XPLANE_NANOMSG_POSITION_H
#define XPLANE_NANOMSG_POSITION_H


#include "DataRefManager.h"

#define FEET_PER_METER 3.28084

struct PosRefs {
  S_DataRefInfo x, y, z;
  S_DataRefInfo vx, vy, vz;
  S_DataRefInfo ax, ay, az;
  S_DataRefInfo lat, lon;
  S_DataRefInfo lat_ref, lon_ref;
  S_DataRefInfo elev, y_agl;  // alt is calculated from elv
  S_DataRefInfo heading, pitch, roll;
  S_DataRefInfo q;
  S_DataRefInfo true_heading, true_pitch, true_roll;
  S_DataRefInfo gs, ias, ias2, tas;
  S_DataRefInfo vh_ind, vh_ind_fpm, vh_ind_fpm2;
  S_DataRefInfo mag_heading, mag_variation;
  S_DataRefInfo alpha, beta, vpath, hpath;
};

// The aircraft position
// See https://developer.x-plane.com/article/movingtheplane/
class Position {
private:
  S_DataRefManager mgr;
  PosRefs refs;

public:
  explicit Position(S_DataRefManager & mgr);

  void update();
  void setPositionFeet(double newLat, double newLon, double newAlt);
  void setPositionMetric(double newLat, double newLon, double newElev);
  void toProtobufData(xplane::Position * p) const;

  double x, y, z;             // meters
  float vx, vy, vz;           // meters/second
  float ax, ay, az;           // meters/second^2
  double lat, lon;            // degrees
  float lat_ref, lon_ref;     // degrees of 0 coordinates
  double elev;                // meters above MSL
  float y_agl;                // meters
  // TODO: check psi vs true_psi, etc
  float pitch, roll, heading; // degrees
  float true_pitch, true_roll, true_heading; // degrees
  float gs, ias, ias2, tas;
  float vh_ind, vh_ind_fpm, vh_ind_fpm2;
  float mag_heading, mag_variation; // degrees
  float alpha, beta, vpath, hpath;
};

using S_Position = std::shared_ptr<Position>;

#endif //XPLANE_NANOMSG_POSITION_H
