/*
    Utility functions to calculate distance between two satellites
    Positions of each satellite are given by sat->pos
    sat->pos is vector of [x, y, z] obtained from SGP4 SDP4 algorithms
    Find distance between two points in a sphere where locations are known
*/


// have not been included in makefile, include when ready

#ifdef HAVE_CONFIG_H
#include <build-config.h>
#endif


#include <glib.h>
#include <glib/gi18n.h>

#include "gtk-sat-data.h"
#include "orbit-tools.h"
#include "calc-dist-two-sat.h"
#include "sat-cfg.h"
#include "sat-log.h"
#include "sgpsdp/sgp4sdp4.h"
#include "time-tools.h"

gdouble dist_calc (sat_t *sat1, sat_t *sat2)
{
    gdouble sat1_posx, sat1_posy, sat1_posz;
    gdouble sat2_posx, sat2_posy, sat2_posz;

    sat1_posx = sat1->pos.x;
    sat1_posy = sat1->pos.y;
    sat1_posz = sat1->pos.z;

    sat2_posx = sat2->pos.x;
    sat2_posy = sat2->pos.y;
    sat2_posz = sat2->pos.z;

    return dist_calc_driver(sat1_posx, sat1_posy, sat1_posz,
                            sat2_posx, sat2_posy, sat2_posz);
}

gdouble dist_calc_driver (gdouble sat1_posx, gdouble sat1_posy, gdouble sat1_posz,
                          gdouble sat2_posx, gdouble sat2_posy, gdouble sat2_posz)
{
    //placeholder
    return 0.0;
}