
#include <XPLMGraphics.h>
#include "Position.h"
#include <cmath>

#define PI 3.14159265358979323846

Position::Position(std::shared_ptr<DataRefManager> & mgr) : mgr(mgr) {
  // See https://developer.x-plane.com/article/movingtheplane/
  // The ones marked with 'W' are writable according to DataRefs.txt,
  // and the ones marked with 'R' are read only.
  // This is not the whole picture: some non-writable ones are
  // written in some circumstances, but may be overwritten by the SIM.
  refs.x       = mgr->get("sim/flightmodel/position/local_x"); // W
  refs.y       = mgr->get("sim/flightmodel/position/local_y"); // W
  refs.z       = mgr->get("sim/flightmodel/position/local_z"); // W

  refs.lat     = mgr->get("sim/flightmodel/position/latitude");  // R
  refs.lon     = mgr->get("sim/flightmodel/position/longitude"); // R
  refs.elev    = mgr->get("sim/flightmodel/position/elevation"); // R
  refs.y_agl   = mgr->get("sim/flightmodel/position/y_agl");     // R

  refs.lat_ref = mgr->get("sim/flightmodel/position/lat_ref");  // R
  refs.lon_ref = mgr->get("sim/flightmodel/position/lon_ref");  // R

  refs.pitch   = mgr->get("sim/flightmodel/position/theta"); // W
  refs.roll    = mgr->get("sim/flightmodel/position/phi");   // W
  refs.heading = mgr->get("sim/flightmodel/position/psi");   // W

  refs.true_pitch   = mgr->get("sim/flightmodel/position/true_theta"); // R
  refs.true_roll    = mgr->get("sim/flightmodel/position/true_phi");   // R
  refs.true_heading = mgr->get("sim/flightmodel/position/true_psi");   // R

  refs.mag_heading  = mgr->get("sim/flightmodel/position/mag_psi");    // R

  refs.q       = mgr->get("sim/flightmodel/position/q");     // W
}

void Position::update() {
  x = refs.x->getDouble();
  y = refs.y->getDouble();
  z = refs.z->getDouble();

  lat = refs.lat->getDouble();
  lon = refs.lon->getDouble();
  elev = refs.elev->getDouble();

  lat_ref = refs.lat_ref->getFloat();
  lon_ref = refs.lon_ref->getFloat();

  alt = elev * FEET_PER_METER;
  y_agl = refs.y_agl->getFloat();

  heading = refs.heading->getFloat();
  pitch = refs.pitch->getFloat();
  roll = refs.roll->getFloat();

  true_heading = refs.true_heading->getFloat();
  true_pitch = refs.true_pitch->getFloat();
  true_roll = refs.true_roll->getFloat();

  mag_heading = refs.mag_heading->getFloat();
}

void Position::setPositionFeet(double newLat, double newLon, double newAlt) {
  setPositionMetric(newLat, newLon, newAlt / FEET_PER_METER);
}

// Moves the airplane around
// See https://developer.x-plane.com/article/movingtheplane/ for the magic
void Position::setPositionMetric(double newLat, double newLon, double newElev) {
  // TODO: Use https://developer.x-plane.com/sdk/XPLMPlanes/#XPLMPlaceUserAtLocation instead

  lat = newLat;
  lon = newLon;
  elev = newElev;
  alt = newElev * FEET_PER_METER;
  XPLMWorldToLocal(lat, lon, elev, &x, &y, &z);
  // TODO: demo code sets floats, but DataRefs.txt says these are doubles
  XPLMSetDatad(refs.x->ref, x);
  XPLMSetDatad(refs.y->ref, y);
  XPLMSetDatad(refs.z->ref, z);
  // According to DataRefs.txt these are read only, but:
  // "To move the plane a LONG way, you must change local_x, local_y
  // and local_z as well as latitude and longitude."
  // "In the future it may not be necessary to write lat/lon and xyz,
  // but it won’t be harmful."
  // https://developer.x-plane.com/article/movingtheplane/
  XPLMSetDatad(refs.lat->ref, lat);
  XPLMSetDatad(refs.lon->ref, lon);
  XPLMSetDatad(refs.elev->ref, elev);

  // The orientation of the plane is in cartesian coordinates. So if you
  // position the plane from latitude 45N to latitude 45S, the plane will
  // be rotated 90 degrees relative to the horizon. This is because the
  // horizon has rotated 90 degrees and the plane has not.
  // The coordinate shift happens after you write your datarefs. So…you’ll
  // need to calculate the change in attitude/rotation yourself (and write
  // the ‘q’ variable – see the SDK tech notes for more on positioning the
  // plane) as you write xyz, anticipating the horizon angle changing.
  // Also from https://developer.x-plane.com/article/movingtheplane/
  XPLMSetDataf(refs.pitch->ref, pitch);
  XPLMSetDataf(refs.roll->ref, roll);
  XPLMSetDataf(refs.heading->ref, heading);
  float q[4];
  float psi = PI / 360 * heading;
  float theta = PI / 360 * pitch;
  float phi = PI / 360 * roll;
  q[0] =  cos(psi) * cos(theta) * cos(phi) + sin(psi) * sin(theta) * sin(phi);
  q[1] =  cos(psi) * cos(theta) * sin(phi) - sin(psi) * sin(theta) * cos(phi);
  q[2] =  cos(psi) * sin(theta) * cos(phi) + sin(psi) * cos(theta) * sin(phi);
  q[3] = -cos(psi) * sin(theta) * sin(phi) + sin(psi) * cos(theta) * cos(phi);
  XPLMSetDatavf(refs.q->ref, q, 0, 4);
}

void Position::toProtobufData(xplane::Position *p) const {
  p->set_x(x);
  p->set_y(y);
  p->set_z(z);

  p->set_lat(lat);
  p->set_lon(lon);

  p->set_lat_ref(lat_ref);
  p->set_lon_ref(lon_ref);

  p->set_elev(elev);
  p->set_alt(alt);
  p->set_y_agl(y_agl);

  p->set_pitch(pitch);
  p->set_roll(roll);
  p->set_heading(heading);

  p->set_true_pitch(true_pitch);
  p->set_true_roll(true_roll);
  p->set_true_heading(true_heading);

  p->set_mag_heading(mag_heading);
}
