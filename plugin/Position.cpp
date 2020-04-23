
#include <XPLMGraphics.h>
#include "Position.h"
#include <cmath>
#include <XPLMPlanes.h>

#define PI 3.14159265358979323846

Position::Position(S_DataRefManager & mgr) : mgr(mgr) {
  // See https://developer.x-plane.com/article/movingtheplane/
  // The ones marked with 'W' are writable according to DataRefs.txt,
  // and the ones marked with 'R' are read only.
  // This is not the whole picture: some non-writable ones are
  // written in some circumstances, but may be overwritten by the SIM.
  refs.x       = mgr->get("sim/flightmodel/position/local_x"); // W
  refs.y       = mgr->get("sim/flightmodel/position/local_y"); // W
  refs.z       = mgr->get("sim/flightmodel/position/local_z"); // W

  refs.vx      = mgr->get("sim/flightmodel/position/local_vx"); // W
  refs.vy      = mgr->get("sim/flightmodel/position/local_vy"); // W
  refs.vz      = mgr->get("sim/flightmodel/position/local_vz"); // W

  refs.ax      = mgr->get("sim/flightmodel/position/local_ax"); // W
  refs.ay      = mgr->get("sim/flightmodel/position/local_ay"); // W
  refs.az      = mgr->get("sim/flightmodel/position/local_az"); // W

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

  refs.gs      = mgr->get("sim/flightmodel/position/groundspeed");
  refs.ias     = mgr->get("sim/flightmodel/position/indicated_airspeed");
  refs.ias2    = mgr->get("sim/flightmodel/position/indicated_airspeed2");
  refs.tas     = mgr->get("sim/flightmodel/position/true_airspeed");
  refs.vh_ind  = mgr->get("sim/flightmodel/position/vh_ind");
  refs.vh_ind_fpm  = mgr->get("sim/flightmodel/position/vh_ind_fpm");
  refs.vh_ind_fpm2 = mgr->get("sim/flightmodel/position/vh_ind_fpm2");
  refs.mag_heading = mgr->get("sim/flightmodel/position/mag_psi");    // R
  refs.alpha   = mgr->get("sim/flightmodel/position/alpha");
  refs.beta    = mgr->get("sim/flightmodel/position/beta");
  refs.vpath   = mgr->get("sim/flightmodel/position/vpath");
  refs.hpath   = mgr->get("sim/flightmodel/position/hpath");
  refs.mag_variation = mgr->get("sim/flightmodel/position/magnetic_variation");    // R

  refs.q       = mgr->get("sim/flightmodel/position/q");     // W
}

void Position::update() {
  x = refs.x->getDouble();
  y = refs.y->getDouble();
  z = refs.z->getDouble();

  vx = refs.vx->getFloat();
  vy = refs.vy->getFloat();
  vz = refs.vz->getFloat();

  ax = refs.ax->getFloat();
  ay = refs.ay->getFloat();
  az = refs.az->getFloat();

  lat = refs.lat->getDouble();
  lon = refs.lon->getDouble();
  elev = refs.elev->getDouble();

  lat_ref = refs.lat_ref->getFloat();
  lon_ref = refs.lon_ref->getFloat();

  y_agl = refs.y_agl->getFloat();

  heading = refs.heading->getFloat();
  pitch = refs.pitch->getFloat();
  roll = refs.roll->getFloat();

  true_heading = refs.true_heading->getFloat();
  true_pitch = refs.true_pitch->getFloat();
  true_roll = refs.true_roll->getFloat();

  gs = refs.gs->getFloat();
  ias = refs.ias->getFloat();
  ias2 = refs.ias2->getFloat();
  tas = refs.tas->getFloat();
  vh_ind = refs.vh_ind->getFloat();
  vh_ind_fpm = refs.vh_ind_fpm->getFloat();
  vh_ind_fpm2 = refs.vh_ind_fpm2->getFloat();
  mag_heading = refs.mag_heading->getFloat();
  alpha = refs.alpha->getFloat();
  beta = refs.beta->getFloat();
  vpath = refs.vpath->getFloat();
  hpath = refs.hpath->getFloat();
  mag_variation = refs.mag_variation->getFloat();
}

void Position::setPositionFeet(double newLat, double newLon, double newAlt) {
  // TODO: REMOVE THIS ONE
  setPositionMetric(newLat, newLon, newAlt / FEET_PER_METER);
}

// Moves the airplane around
void Position::setPositionMetric(double newLat, double newLon, double newElev) {
  // TODO: Use https://developer.x-plane.com/sdk/XPLMPlanes/#XPLMPlaceUserAtLocation instead
  // TODO: allow passing new heading and gs
  // This call can take a long time if scenery needs to be loaded!
  XPLMPlaceUserAtLocation(newLat, newLon, float(newElev), heading, gs);

  lat = newLat;
  lon = newLon;
  elev = newElev;

  /*
  // Old method described in https://developer.x-plane.com/article/movingtheplane/
  // The newer X-Plane versions (XPLM 300+) have a XPLMPlaceUserAtLocation() call
  // for this that does it better (scenery load screen, engine running, etc)

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
  */
}

void Position::toProtobufData(xplane::Position *p) const {
  {
    auto m = p->mutable_pos();
    m->set_x(x);
    m->set_y(y);
    m->set_z(z);
  }
  {
    auto m = p->mutable_speed();
    m->set_x(vx);
    m->set_y(vy);
    m->set_z(vz);
  }
  {
    auto m = p->mutable_accel();
    m->set_x(ax);
    m->set_y(ay);
    m->set_z(az);
  }
  {
    auto m = p->mutable_glob();
    m->set_lat(lat);
    m->set_lon(lon);
    m->set_elev(elev);
    m->set_y_agl(y_agl);
  }
  {
    auto m = p->mutable_reference();
    m->set_lat(lat_ref);
    m->set_lon(lon_ref);
  }
  {
    auto m = p->mutable_attitude();
    m->set_pitch(pitch);
    m->set_roll(roll);
    m->set_heading(heading);
  }
  {
    auto m = p->mutable_true_attitude();
    m->set_pitch(true_pitch);
    m->set_roll(true_roll);
    m->set_heading(true_heading);
  }
  // TODO: attitude_speed
  // TODO: attitude_accel
  // TODO: angular_momentum
  {
    auto m = p->mutable_instruments();
    m->set_gs(gs);
    m->set_ias(ias);
    m->set_ias2(ias2);
    m->set_tas(tas);
    m->set_vh_ind(vh_ind);
    m->set_vh_ind_fpm(vh_ind_fpm);
    m->set_vh_ind_fpm2(vh_ind_fpm2);
    m->set_mag_heading(mag_heading);
    m->set_alpha(alpha);
    m->set_beta(beta);
    m->set_vpath(vpath);
    m->set_hpath(hpath);
    m->set_mag_variation(mag_variation);
  }
  // TODO: q[4]
}
