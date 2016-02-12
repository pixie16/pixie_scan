/** \file ChanEvent.cpp
 * \brief A Class to define what a channel event is
 */

#include "ChanEvent.hpp"

bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b) {
    return (a->GetCorrectedTime() < b->GetCorrectedTime());
}

bool CompareTime(const ChanEvent *a, const ChanEvent *b){
    return (a->GetTime() < b->GetTime());
}

void ChanEvent::ZeroNums() {
	event->Clear();
}

unsigned long ChanEvent::GetQdcValue(int i) const {
    return event->GetQdcValue(i);
}

const Identifier& ChanEvent::GetChanID() const {
    return DetectorLibrary::get()->at(event->modNum, event->chanNum);
}

int ChanEvent::GetID() const {
    return DetectorLibrary::get()->GetIndex(event->modNum, event->chanNum);
}

//! [Zero Channel]
void ChanEvent::ZeroVar() {
    ZeroNums();
    trace.clear();
}
//! [Zero Channel]
