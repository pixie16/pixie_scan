/** \file NaIProcessor.hpp
 * 
 * 
 */

#ifndef __PINPROCESSOR_HPP_
#define __PINPROCESSOR_HPP_

#include "EventProcessor.hpp"

class PINProcessor : public EventProcessor
{
 public:
  PINProcessor();
  virtual void DeclarePlots(void);
  virtual bool Process(RawEvent &rEvent);
private:
  struct PINData {
    void Clear(void);
  } data;
};

#endif // __PINPROCESSOR_HPP_
