/** \file SHECorrelator.hpp
 *
 */

#ifndef __JAEACORRELATOR_HPP_
#define __JAEACORRELATOR_HPP_

#include <vector>
#include <deque> 
#include <sstream>

#include "Plots.hpp"
#include "Globals.hpp"
#include "ChanEvent.hpp"
#include "Messenger.hpp"
#include "WalkCorrector.hpp"
#include "Calibrator.hpp"
#include "DammPlotIds.hpp"


enum JAEAEventType {
    alpha_jaea,
    heavyIon_jaea,
    fission_jaea,
    lightIon_jaea,
    unknown_jaea
};

class JAEAEvent {
    public:
        JAEAEvent();
        JAEAEvent(double energy, double time, int mwpc, double mwpcTime, 
		  bool has_beam, bool has_veto, bool has_escape,
		  JAEAEventType type);
  
        ~JAEAEvent() {}

        double get_energy() const {
            return energy_;
        }
        double get_time() const {
            return time_;
        }
        int get_mwpc() const {
            return mwpc_;
        }
	double get_mwpcTime() const {
            return mwpcTime_;
        }
        bool get_beam() const {
            return has_beam_;
        }
        bool get_veto() const {
            return has_veto_;
        }
        bool get_escape() const {
            return has_escape_;
        }
        JAEAEventType get_type() const {
            return type_;
        }

        void set_energy(double energy) {
            energy_  = energy;
        }
        void set_time(double time) {
            time_ = time;
        }
        void set_mwpc(int mwpc) {
            mwpc_ = mwpc;
        }
        void set_mwpcTime(double mwpcTime) {
            mwpcTime_ = mwpcTime;
        }
        void set_beam(bool has_beam) {
            has_beam_ = has_beam;
        }
        void set_veto(bool has_veto) {
            has_veto_ = has_veto;
        }
        void set_escape(bool has_escape) {
            has_escape_ = has_escape;
        }
        void set_type(JAEAEventType type) {
            type_ = type;
        }

    private:
        /** Total (reconstructed) energy, may include escape **/
        double energy_;
        /** Shortest time of all subevents (e.g. back and front) */
        double time_;
        /** Number of MWPC chambers hits */
        int mwpc_;
	/** Time MWPC of hits */
        double mwpcTime_;
        /** Veto hit flag **/
        bool has_veto_;
        /** Beam on flag **/
        bool has_beam_;
        /** If reconstructed energy includes escape **/
        bool has_escape_;
        /** Type of event decided by Correlator **/
        JAEAEventType type_;
};


class JAEACorrelator {
    public:
        JAEACorrelator(int size_x, int size_y);
        ~JAEACorrelator();
        bool add_event(JAEAEvent& event, int x, int y, Plots& histo);
        void human_event_info(JAEAEvent& event, std::stringstream& ss, double clockStart);
  

    private:
        int size_x_;
        int size_y_;
        std::deque<JAEAEvent>** pixels_;

        bool flush_chain(int x, int y, Plots& histo);
};


#endif
