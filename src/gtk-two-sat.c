#ifdef HAVE_CONFIG_H
#include <build-config.h>
#endif
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "config-keys.h"
#include "gpredict-utils.h"
#include "gtk-sat-data.h"
#include "gtk-sat-popup-common.h"
#include "gtk-two-sat.h"
#include "locator.h"
#include "mod-cfg-get-param.h"
#include "orbit-tools.h"
#include "predict-tools.h"
#include "sat-cfg.h"
#include "sat-info.h"
#include "sat-log.h"
#include "sat-vis.h"
#include "sgpsdp/sgp4sdp4.h"
#include "sat-pass-dialogs.h"
#include "time-tools.h"

/* Column titles indexed with column symb. refs. */
const gchar    *TWO_SAT_FIELD_TITLE[TWO_SAT_FIELD_NUMBER] = {
    N_("Azimuth"),
    N_("Elevation"),
    N_("Direction"),
    N_("Right Asc."),
    N_("Declination"),
    N_("Slant Range"),
    N_("Range Rate"),
    N_("Next Event"),
    N_("Next AOS"),
    N_("Next LOS"),
    N_("SSP Lat."),
    N_("SSP Lon."),
    N_("SSP Loc."),
    N_("Footprint"),
    N_("Altitude"),
    N_("Velocity"),
    N_("Doppler@100M"),
    N_("Sig. Loss"),
    N_("Sig. Delay"),
    N_("Mean Anom."),
    N_("Orbit Phase"),
    N_("Orbit Num."),
    N_("Visibility"),
    N_("SKR")
};

/* Column title hints indexed with column symb. refs. */
const gchar    *TWO_SAT_FIELD_HINT[TWO_SAT_FIELD_NUMBER] = {
    N_("Azimuth of the satellite"),
    N_("Elevation of the satellite"),
    N_("Direction of the satellite"),
    N_("Right Ascension of the satellite"),
    N_("Declination of the satellite"),
    N_("The range between satellite and observer"),
    N_("The rate at which the Slant Range changes"),
    N_("The time of next AOS or LOS"),
    N_("The time of next AOS"),
    N_("The time of next LOS"),
    N_("Latitude of the sub-satellite point"),
    N_("Longitude of the sub-satellite point"),
    N_("Sub-Satellite Point as Maidenhead grid square"),
    N_("Diameter of the satellite footprint"),
    N_("Altitude of the satellite"),
    N_("Tangential velocity of the satellite"),
    N_("Doppler Shift @ 100MHz"),
    N_("Signal loss @ 100MHz"),
    N_("Signal Delay"),
    N_("Mean Anomaly"),
    N_("Orbit Phase"),
    N_("Orbit Number"),
    N_("Visibility of the satellite"),
    N_("Secret Key Rate")
};

static GtkBoxClass *parent_class = NULL;

static void gtk_two_sat_destroy(GtkWidget * widget)
{
    GtkTwoSat   *ssat = GTK_TWO_SAT(widget);
    sat_t       *sat1 = SAT(g_slist_nth_data(ssat->sats, ssat->selected1));
    sat_t       *sat2 = SAT(g_slist_nth_data(ssat->sats, ssat->selected2));

    if (sat1 != NULL || sat2 != NULL)
    {
        g_key_file_set_integer(ssat->cfgdata, MOD_CFG_TWO_SAT_SECTION,
                               MOD_CFG_TWO_SAT_SELECT_FIRST, sat1->tle.catnr);
        g_key_file_set_integer(ssat->cfgdata, MOD_CFG_SINGLE_SAT_SECTION,
                               MOD_CFG_TWO_SAT_SELECT_SECOND, sat2->tle.catnr);
    }

    (*GTK_WIDGET_CLASS(parent_class)->destroy) (widget);
}

static void gtk_two_sat_class_init(GtkTwoSatClass * class, gpointer class_data)
{
    GtkWidgetClass *widget_class;

    (void)class_data;

    widget_class = (GtkWidgetClass *) class;
    widget_class->destroy = gtk_two_sat_destroy;
    parent_class = g_type_class_peek_parent(class);
}

static void gtk_two_sat_init(GtkTwoSat * list, gpointer g_class)
{
    (void)list;
    (void)g_class;
}

// Update a field in the GtkTwoSat View
// IMPORTANT: when calling, include param for which sat to update
// selection == 1 means first satellite to be udpated
// selection == 2 means second satellite to be updated
static void update_field(GtkTwoSat * tsat, guint i, guint selection)
{
    sat_t      *sat;
    gchar      *buff = NULL;
    gchar       tbuf[TIME_FORMAT_MAX_LENGTH];
    gchar       hmf = ' ';
    gdouble     number;
    gint        retcode;
    gchar      *fmtstr;
    gchar      *alstr;
    sat_vis_t   vis;
    gdouble     skr;

    // Make some sanity checks
    if(tsat->labels[i] == NULL)
    {
        sat_log_log(SAT_LOG_LEVEL_ERROR,
                    _("%s:%d: Can not update invisible field (I:%d F:%d)"),
                    __FILE__, __LINE__, i, ssat->flags);
        return;
    }

    // Get selected satellite
    if (selection == 1)
    {
        sat = SAT(g_slist_nth_data(tsat->sats, tsat->selected1));
    }
    else if (selection == 2)
    {
        sat = SAT(g_slist_nth_data(tsat->sats, tsat->selected2));
    }
    else
    {
        sat_log_log(SAT_LOG_LEVEL_ERROR, "%s: %d - selection is invalid (must be either 1 or 2).",
                    __FILE__, __LINE__);
        return;
    }
    if (!sat)
    {
        sat_log_log(SAT_LOG_LEVEL_ERROR, "%s: %d - Cannot update non-existing sat",
                    __FILE__, __LINE__);
        return;
    }

    // Update requested field
    switch (i)
    {
    case TWO_SAT_FIELD_AZ:
        buff = g_strdup_printf("%6.2f\302\260", sat->az);
        break;
    case TWO_SAT_FIELD_EL:
        buff = g_strdup_printf("%6.2f\302\260", sat->el);
        break;
    case TWO_SAT_FIELD_DIR:
        if (sat->otype == ORBIT_TYPE_GEO)
        {
            buff = g_strdup("Geostationary");
        }
        else if (decayed(sat))
        {
            buff = g_strdup("Decayed");
        }
        else if (sat->range_rate > 0.0)
        {
            // Receding
            buff = g_strdup("Receding");
        }
        else if (sat->range_rate < 0.0)
        {
            // Approaching
            buff = g_strdup("Approaching");
        }
        else
        {
            buff = g_strdup("Approaching");
        }
        break;
    case TWO_SAT_FIELD_RA:
        buff = g_strdup_printf("%6.2f\302\260", sat->ra);
        break;
    case TWO_SAT_FIELD_DEC:
        buff = g_strdup_printf("%6.2f\302\260", sat->dec);
        break;
    case TWO_SAT_FIELD_RANGE:
        if (sat_cfg_get_bool(SAT_CFG_BOOL_USE_IMPERIAL))
            buff = g_strdup_printf("%.3f mi/sec", KM_TO_MI(sat->range_rate));
        else
            buff = g_strdup_printf("%.3f km/sec", sat->range_rate);
        break;
    case TWO_SAT_FIELD_NEXT_EVENT:
        if(sat->aos > sat->lows)
        {
            // Next event is LOS
            number = sat->los;
            alstr = g_strdup("LOS: ");
        }
        else
        {
            // Next event is AOS
            number = sat->aos;
            alstr = g_strdup("AOS: ");
        }
        if (number > 0.0)
        {
            // Format the number
            fmtstr = sat_cfg_get_str(SAT_CFG_STR_TIME_FORMAT);
            daynum_to_str(tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, number);

            g_free(fmtstr);

            buff = g_strconcat(alstr, tbuf, NULL);
        }
        else
        {
            buff = g_strdup(_("N/A"));
        }
        g_free(alstr);
        break;
    case TWO_SAT_FIELD_AOS:
        if (sat->aos > 0.0)
        {
            // Format the number
            fmtstr = sat_cfg_get_str(SAT_CFG_STR_TIME_FORMAT);
            daynum_to_str(tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, sat->aos);
            g_free(fmtstr);
            buff = g_strdup(tbuf);
        }
        else
        {
            buff = g_strdup(_("N/A"));
        }
        break;
    case TWO_SAT_FIELD_LOS:
        if (sat->los > 0.0)
        {
            fmtstr = sat_cfg_get_str(SAT_CFG_STR_TIME_FORMAT);
            daynum_to_str(tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, sat->los);
            g_free(fmtstr);
            buff = g_strdup(tbuf);
        }
        else
        {
            buff = g_strdup(_("N/A"));
        }
        break;
    case TWO_SAT_FIELD_LAT:
        number = sat->ssplat;
        if (sat_cfg_get_bool(SAT_CFG_BOOL_USE_NSEW))
        {
            if (number < 0.00)
            {
                number = -number;
                hmf = 'S';
            }
            else
            {
                hmf = 'N';
            }
        }
        buff = g_strdup_printf("%.2f\302\260%c", number, hmf);
        break;
    case TWO_SAT_FIELD_LON:
        number = sat->ssplon;
        if (sat_cfg_get_bool(SAT_CFG_BOOL_USE_NSEW))
        {
            if (number < 0.00)
            {
                number = -number;
                hmf = 'W';
            }
            else
            {
                hmf = 'E';
            }
        }
        buff = g_strdup_printf("%.2f\302\260%c", number, hmf);
        break;
    case TWO_SAT_FIELD_SSP:
        // SSP Locator
        buff = g_try_malloc(7);
        retcode = longlat2locator(sat->ssplon, sat->ssplat, buff, 3);
        if (retcode == RIG_OK)
        {
            buff[6] = '\0';
        }
        else
        {
            g_free(buff);
            buff = NULL;
        }
        break;
    case TWO_SAT_FIELD_FOOTPRINT:
        if (sat_cfg_get_bool(SAT_CFG_BOOL_USE_IMPERIAL))
        {
            buff = g_strdup_printf("%.0f mi", KM_TO_MI(sat->footprint));
        }
        else
        {
            buff = g_strdup_printf("%.0f km", sat->footprint);
        }
        break;
    case TWO_SAT_FIELD_ALT:
        if (sat_cfg_get_bool(SAT_CFG_BOOL_USE_IMPERIAL))
        {
            buff = g_strdup_printf("%.0f mi", KM_TO_MI(sat->alt));
        }
        else
        {
            buff = g_strdup_printf("%.0f km", sat->alt);
        }
        break;
    case TWO_SAT_FIELD_VEL:
        if (sat_cfg_get_bool(SAT_CFG_BOOL_USE_IMPERIAL))
        {
            buff = g_strdup_printf("%.3f mi/sec", KM_TO_MI(sat->velo));
        }
        else
        {
            buff = g_strdup_printf(".3f km/sec", sat->velo);
        }
        break;
    case TWO_SAT_FIELD_DOPPLER:
        number = -100.0e06 * (sat->range_rate / 299792.4580);   // Hz
        buff = g_strdup_printf("%.0f Hz", number);
        break;
    case TWO_SAT_FIELD_LOSS:
        number = 72.4 + 20.0 * log10(sat->range);       // dB
        buff = g_strdup_printf("%.2f dB", number);
        break;
    case TWO_SAT_FIELD_DELAY:
        number = sat->range / 299.7924580;      // msec 
        buff = g_strdup_printf("%.2f msec", number);
        break;
    case TWO_SAT_FIELD_MA:
        buff = g_strdup_printf("%.2f\302\260", sat->ma);
        break;
    case TWO_SAT_FIELD_PHASE:
        buff = g_strdup_printf("%.2f\302\260", sat->phase);
        break;
    case TWO_SAT_FIELD_ORBIT:
        buff = g_strdup_printf("%ld", sat->orbit);
        break;
    case TWO_SAT_FIELD_VISIBILITY:
        vis = get_sat_vis(sat, ssat->qth, sat->jul_utc);
        buff = vis_to_str(vis);
        break;
    case TWO_SAT_FIELD_SKR:
        // using this field for finding out what is xyz in sgp first
        // placeholder
        buff = g_strdup_printf("x %lf y %lf z %lf mag %lf", sat->pos.x, sat->pos.y, sat->pos.z, sat->pos.w);
        break;
    default:
        sat_log_log(SAT_LOG_LEVEL_ERROR,
                    _("%s:%d: Invalid field number (%d)"),
                    __FILE__, __LINE__, i);
        break;
    }

    if (buff != NULL)
    {
        gtk_label_set_text(GTK_LABEL(ssat->labels[i]), buff);
        /*
        sat_log_log(SAT_LOG_LEVEL_DEBUG,
                    ("%s %s: current buff is %s, writing to %d case"), __FILE__,
                    buff, i);
        */        
        g_free(buff);
    }
}

static gint sat_name_compare (sat_t * a, sat_t * b)
{
    return gpredict_strcmp(a->nickname, b->nickname);
}

// Copy satellite from hash table to singly linked list
static void store_sats(gpointer key, gpointer value, gpointer user_data)
{
    GtkTwoSat      *two_sat = GTK_TWO_SAT(user_data);
    sat_t          *sat = SAT(value);

    (void)key;

    two_sat->sats = g_slist_insert_sorted(two_sat->sats, sat,
                                          (GCompareFunc) sat_name_compare);
}

static void Calculate_RADec(sat_t * sat, qth_t * qth, obs_astro_t * obs_set)
{
    /* Reference:  Methods of Orbit Determination by  */
    /*                Pedro Ramon Escobal, pp. 401-402 */

    double          phi, theta, sin_theta, cos_theta, sin_phi, cos_phi,
        az, el, Lxh, Lyh, Lzh, Sx, Ex, Zx, Sy, Ey, Zy, Sz, Ez, Zz,
        Lx, Ly, Lz, cos_delta, sin_alpha, cos_alpha;
    geodetic_t      geodetic;

    geodetic.lon = qth->lon * de2ra;
    geodetic.lat = qth->lat * de2ra;
    geodetic.alt = qth->alt / 1000.0;
    geodetic.theta = 0;

    az = sat->az * de2ra;
    el = sat->el * de2ra;
    phi = geodetic.lat;
    theta = FMod2p(ThetaG_JD(sat->jul_utc) + geodetic.lon);
    sin_theta = sin(theta);
    cos_theta = cos(theta);
    sin_phi = sin(phi);
    cos_phi = cos(phi);
    Lxh = -cos(az) * cos(el);
    Lyh = sin(az) * cos(el);
    Lzh = sin(el);
    Sx = sin_phi * cos_theta;
    Ex = -sin_theta;
    Zx = cos_theta * cos_phi;
    Sy = sin_phi * sin_theta;
    Ey = cos_theta;
    Zy = sin_theta * cos_phi;
    Sz = -cos_phi;
    Ez = 0;
    Zz = sin_phi;
    Lx = Sx * Lxh + Ex * Lyh + Zx * Lzh;
    Ly = Sy * Lxh + Ey * Lyh + Zy * Lzh;
    Lz = Sz * Lxh + Ez * Lyh + Zz * Lzh;
    obs_set->dec = ArcSin(Lz);  /* Declination (radians) */
    cos_delta = sqrt(1 - Sqr(Lz));
    sin_alpha = Ly / cos_delta;
    cos_alpha = Lx / cos_delta;
    obs_set->ra = AcTan(sin_alpha, cos_alpha);  /* Right Ascension (radians) */
    obs_set->ra = FMod2p(obs_set->ra);
}


// Select first satellite from drop down menu
static void select_first_satellite(GtkWidget * menuitem, gpointer data)
{
    GtkTwoSat      *ssat = GTK_TWO_SAT(data);
    guint           i =
        GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(menuitem), "index"));
    gchar          *title;
    sat_t          *sat;

    // There are many "ghost"-triggering of this signal, but we only need to
    // make a new selection when the received menuitem is selected
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
    {
        ssat->selected1 = i;

        sat = SAT(g_slist_nth_data(ssat->sats, i));

        title = g_markup_printf_escaped("<b>First Satellite: %s</b>", sat->nickname);
        gtk_label_set_markup(GTK_LABEL(ssat->header1), title);
        g_free(title);
    }
}

// Select second satellite from drop down menu
static void select_second_satellite(GtkWidget * menuitem, gpointer data)
{
    GtkTwoSat      *ssat = GTK_TWO_SAT(data);
    guint           i =
        GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(menuitem), "index"));
    gchar          *title;
    sat_t          *sat;

    // There are many "ghost"-triggering of this signal, but we only need to
    // make a new selection when the received menuitem is selected
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)))
    {
        ssat->selected2 = i;

        sat = SAT(g_slist_nth_data(ssat->sats, i));

        title = g_markup_printf_escaped("<b>Second Satellite: %s</b>", sat->nickname);
        gtk_label_set_markup(GTK_LABEL(ssat->header2), title);
        g_free(title);
    }
}