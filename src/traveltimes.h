#pragma once

#include <Rcpp.h>

#include "csa.h"

// ----- debugging output START -----
//#define DEBUG
#define DEPARTURE_STATION 32290
#define ARRIVAL_STATION 32291
#define DEPARTURE_TIME_MIN 29340
#define DEPARTURE_TIME_MAX 30000

#ifdef DEBUG
#define DEBUGMSG(msg, depstn, arrstn, deptime) \
    if ((depstn) == DEPARTURE_STATION && \
            (arrstn) == ARRIVAL_STATION && \
            (deptime) >= DEPARTURE_TIME_MIN && \
            (deptime) <= DEPARTURE_TIME_MAX) \
    Rcpp::Rcout << msg << std::endl;
#else
#define DEBUGMSG(msg, depstn, arrstn, deptime) do{}while(0)
#endif

#ifdef DEBUG
#define DEBUGMSGTR(msg, trstn, deptime) \
    if ((trstn) == DEPARTURE_STATION && \
            (deptime) >= DEPARTURE_TIME_MIN && \
            (deptime) <= DEPARTURE_TIME_MAX) \
    Rcpp::Rcout << msg << std::endl;
#else
#define DEBUGMSGTR(msg, trstn, deptime) do{}while(0)
#endif
// ----- debugging output END -----

class Iso
{
    private:

        struct OneCon {
            size_t prev_stn;
            int departure_time,
                arrival_time,
                trip,
                ntransfers,
                initial_depart;
        };

        struct ConVec {
            std::vector <OneCon> convec;
        };

    public:

        std::vector <bool> is_end_stn;
        std::vector <int> earliest_departure;

        std::vector <ConVec> connections;

        Iso (const size_t n) {
            is_end_stn.resize (n, false);
            earliest_departure.resize (n, INFINITE_INT);
            connections.resize (n);
        }

        const size_t extend (const size_t n) {
            const size_t s = connections [n].convec.size () + 1L;

            connections [n].convec.resize (s);

            connections [n].convec.back ().prev_stn = INFINITE_INT;
            connections [n].convec.back ().departure_time = INFINITE_INT;
            connections [n].convec.back ().arrival_time = INFINITE_INT;
            connections [n].convec.back ().trip = INFINITE_INT;
            connections [n].convec.back ().ntransfers = 0L;
            connections [n].convec.back ().initial_depart = INFINITE_INT;

            return s;
        }

};

struct BackTrace
{
    std::vector <int> trip, end_station, end_times;
};

namespace iso {

void trace_forward_iso (
        Iso & iso,
        const int & start_time,
        const int & end_time,
        const std::vector <size_t> & departure_station,
        const std::vector <size_t> & arrival_station,
        const std::vector <size_t> & trip_id,
        const std::vector <int> & departure_time,
        const std::vector <int> & arrival_time,
        const std::unordered_map <size_t, std::unordered_map <size_t, int> > & transfer_map,
        const std::unordered_set <size_t> & start_stations_set,
        const bool & minimise_transfers);

bool fill_one_iso (
        const size_t &departure_station,
        const size_t &arrival_station,
        const size_t &trip_id,
        const int &departure_time,
        const int &arrival_time,
        const int &isochrone,
        const bool &is_start_stn,
        const bool &minimise_transfers,
        Iso &iso);

void trace_forward_traveltimes (
        Iso & iso,
        const int & start_time,
        const std::vector <size_t> & departure_station,
        const std::vector <size_t> & arrival_station,
        const std::vector <size_t> & trip_id,
        const std::vector <int> & departure_time,
        const std::vector <int> & arrival_time,
        const std::unordered_map <size_t, std::unordered_map <size_t, int> > & transfer_map,
        const std::unordered_set <size_t> & start_stations_set,
        const bool & minimise_transfers,
        const double & prop_stops);

void fill_one_transfer (
        const size_t &departure_station,
        const size_t &arrival_station,
        const int &arrival_time,
        const size_t &trans_dest,
        const int &trans_duration,
        const int &isochrone,
        const bool &minimise_transfers,
        Iso &iso);

int find_actual_end_time (
        const size_t &n,
        const std::vector <int> &departure_time,
        const std::vector <size_t> &departure_station,
        const std::unordered_set <size_t> &start_stations_set,
        const int &start_time,
        const int &end_time
        );

void make_transfer_map (
    std::unordered_map <size_t, std::unordered_map <size_t, int> > &transfer_map,
    const std::vector <size_t> &trans_from,
    const std::vector <size_t> &trans_to,
    const std::vector <int> &trans_time
        );

size_t trace_back_first (
        const Iso & iso,
        const size_t & stn
        );

size_t trace_back_prev_index (
        const Iso & iso,
        const size_t & stn,
        const size_t & departure_time,
        const int & trip_id,
        const bool &minimise_transfers
        );

bool update_best_connection (
        const int & this_initial,
        const int & latest_initial,
        const int & this_transfers,
        const int & min_transfers,
        const bool &minimise_transfers
        );

const bool is_transfer_in_isochrone (
        const Iso & iso,
        const size_t & station,
        const int & transfer_time,
        const int & isochrone
        );

const bool is_transfer_connected (
        const Iso & iso,
        const size_t & station,
        const int & transfer_time
        );

const bool is_start_stn (
    const std::unordered_set <size_t> &start_stations_set,
    const size_t &stn);

const bool arrival_already_visited (
        const Iso & iso,
        const size_t & departure_station,
        const size_t & arrival_station);

// The only two Rcpp functions:
Rcpp::List trace_back_isochrones (
        const Iso & iso,
        const bool &minimise_transfers
        );

Rcpp::IntegerMatrix trace_back_traveltimes (
        const Iso & iso,
        const bool &minimise_transfers
        );

// but most work done in this pure C++ fn:
void trace_back_one_stn (
        const Iso & iso,
        BackTrace & backtrace,
        const size_t & end_stn,
        const bool &minimise_transfers
        );


} // end namespace iso

// ---- isochrone.cpp
Rcpp::List rcpp_isochrone (
        Rcpp::DataFrame timetable,
        Rcpp::DataFrame transfers,
        const size_t nstations,
        const std::vector <size_t> start_stations,
        const int start_time,
        const int end_time,
        const bool minimise_transfers);

Rcpp::IntegerMatrix rcpp_traveltimes (Rcpp::DataFrame timetable,
        Rcpp::DataFrame transfers,
        const size_t nstations,
        const std::vector <size_t> start_stations,
        const int start_time,
        const int end_time,
        const bool minimise_transfers,
        const int cutoff);
