// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// EO.h
// (c) GeNeura Team 1998
//-----------------------------------------------------------------------------

#ifndef EO_H
#define EO_H

//-----------------------------------------------------------------------------

#include <stdexcept>  // runtime_error 
#include <eoObject.h>
#include <eoPersistent.h>

//-----------------------------------------------------------------------------
/** EO is a base class for evolvable objects, that is, the subjects of evolution.
    EOs have only got a fitness, which at the same time needs to be only an object with the
    operation less than (<) defined. Fitness says how good is the object; evolution or change
    of these objects is left to the genetic operators. A fitness less than another means a 
    worse fitness, in whatever the context; thus, fitness is always maximized; although 
    it can be minimized with a proper definition of the < operator.\\
    The fitness object must have, besides an void ctor, a copy ctor.
*/
template<class F> class EO: public eoObject, public eoPersistent
{
public:
  typedef F Fitness;
  
  /** Default constructor.
      Fitness must have a ctor which takes 0 as a value; we can not use void ctors here
      since default types like float have no void initializer. VC++ allows it, but gcc does not
  */
  EO(): repFitness(0), invalidFitness(true) {}

  /** Ctor from stream.
      Fitness must have defined the lecture from an istream.
  */
  EO( istream& _is ) {
    _is >> repFitness;
    invalidFitness = false;
  };

  /// Copy ctor
  EO( const EO& _eo ): repFitness( _eo.repFitness ), invalidFitness( _eo.invalidFitness ) {};
  
  /// Virtual dtor
  virtual ~EO() {};

  /// Return fitness value.
  Fitness fitness() const
    {
      if (invalid())
	//throw runtime_error("invalid fitness");
	cout << "invalid fitness" << endl;
      return repFitness;
    }
  
  // Set fitness as invalid.
  void invalidate() { invalidFitness = true; }
  
  /** Set fitness. At the same time, validates it.
   *  @param _fitness New fitness value.
   */
  void fitness(const Fitness& _fitness)
    { 
      repFitness = _fitness;
      invalidFitness = false;
    }
  
  /** Return true If fitness value is invalid, false otherwise.
   *  @return true If fitness is invalid.
   */
  bool invalid() const { return invalidFitness; }
  
  /** Returns true if 
      @return true if the fitness is higher
   */
  bool operator<(const EO& _eo2) const {  return fitness() < _eo2.fitness();}

  /// Methods inherited from eoObject
  //@{
  
  /** Return the class id. 
   *  @return the class name as a string
   */
  virtual string className() const { return "EO"; };
  
  /**
   * Read object.\\
   * Calls base class, just in case that one had something to do. The read and print 
   * methods should be compatible and have the same format. In principle, format is 
   * "plain": they just print a number
   * @param _is a istream.
   * @throw runtime_exception If a valid object can't be read.
   */
  virtual void readFrom(istream& _is) {
    _is >> repFitness;
    invalidFitness = false;
  }
  
  /**
   * Write object. It's called printOn since it prints the object _on_ a stream.
   * @param _os A ostream.
   */
  virtual void printOn(ostream& _os) const {
    _os << repFitness << endl;
  }
  
  //@}
  
private:
  Fitness repFitness;   // value of fitness for this chromosome
  bool invalidFitness;  // true if the value of fitness is invalid
};

//-----------------------------------------------------------------------------

#endif EO_H
