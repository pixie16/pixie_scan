/** \file NaIProcessor.hpp
 * 
 * 
 */

#ifndef __NAIPROCESSOR_HPP_
#define __NAIPROCESSOR_HPP_

#include "EventProcessor.hpp"

class NaIProcessor : public EventProcessor
{
 public:
  NaIProcessor();
  virtual void DeclarePlots(void);
  virtual bool Process(RawEvent &rEvent);
private:
  struct NaIData {
    void Clear(void);
  } data;
};

#endif // __NAIPROCESSOR_HPP_
