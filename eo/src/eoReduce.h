// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// eoReduce.h
//   Base class for population-merging classes
// (c) GeNeura Team, 1998
/* 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Contact: todos@geneura.ugr.es, http://geneura.ugr.es
 */
//-----------------------------------------------------------------------------

#ifndef eoReduce_h
#define eoReduce_h

//-----------------------------------------------------------------------------

#include <iostream>

// EO includes
#include <eoPop.h>     // eoPop
#include <eoFunctor.h>  // eoReduce
#include <utils/selectors.h>

/**
 * eoReduce: .reduce the new generation to the specified size
   At the moment, limited to truncation - with 2 different methods,
   one that sorts the whole population, and one that repeatidely kills 
   the worst. Ideally, we should be able to choose at run-time!!!
*/

template<class EOT> class eoReduce: public eoBF<eoPop<EOT>&, unsigned, void>
{};

/** truncation method using sort */
template <class EOT> class eoTruncate : public eoReduce<EOT>
{
    void operator()(eoPop<EOT>& _newgen, unsigned _newsize)
    {
        if (_newgen.size() == _newsize)
            return;
	if (_newgen.size() < _newsize)
	  throw std::logic_error("eoTruncate: Cannot truncate to a larger size!\n");

        _newgen.nth_element(_newsize);
        _newgen.resize(_newsize);
    }
};

/** 
EP truncation method (some global stochastic tournament +  sort)
Softer selective pressure than pure truncate
*/
template <class EOT> class eoEPReduce : public eoReduce<EOT>
{
public:
typedef typename EOT::Fitness Fitness; 

  eoEPReduce(unsigned _t_size):
    t_size(_t_size)
  {
    if (t_size < 2)
      { // warning, error?
	t_size = 2;
      }
  }

  /// helper struct for comparing on pairs
  typedef pair<float, eoPop<EOT>::iterator>  EPpair;
      struct Cmp {
          bool operator()(const EPpair a, const EPpair b) const
            { return b.first < a.first; }
      };


    void operator()(eoPop<EOT>& _newgen, unsigned _newsize)
    {
      unsigned int presentSize = _newgen.size();
        if (presentSize == _newsize)
            return;
	if (presentSize < _newsize)
	  throw std::logic_error("eoTruncate: Cannot truncate to a larger size!\n");
	vector<EPpair> scores(presentSize);
	for (unsigned i=0; i<presentSize; i++)
	  {
	    scores[i].second = _newgen.begin()+i;
	    Fitness fit = _newgen[i].fitness();
	    for (unsigned itourn = 0; itourn < t_size; ++itourn)
	      {
		const EOT & competitor = _newgen[rng.random(presentSize)];
		if (fit > competitor.fitness())
		  scores[i].first += 1;
		else if (fit == competitor.fitness())
		  scores[i].first += 0.5;
	      }
	  }
	// now we have the scores
	typename vector<EPpair>::iterator it = scores.begin() + _newsize;
	std::nth_element(scores.begin(), it, scores.end(), Cmp());
	it = scores.begin() + _newsize;	// just in case ???
	while (it < scores.end())
	  _newgen.erase(it->second);
    }
private:
  unsigned t_size;
};

/** a truncate class that does not sort, but repeatidely kills the worse.
To be used in SSGA-like replacements (e.g. see eoSSGAWorseReplacement)
*/
template <class EOT> 
class eoLinearTruncate : public eoReduce<EOT>
{
  void operator()(eoPop<EOT>& _newgen, unsigned _newsize)
  {
    unsigned oldSize = _newgen.size();
    if (oldSize == _newsize)
      return;
    if (oldSize < _newsize)
      throw std::logic_error("eoLinearTruncate: Cannot truncate to a larger size!\n");
    for (unsigned i=0; i<oldSize - _newsize; i++)
      {
	eoPop<EOT>::iterator it = _newgen.it_worse_element();
	_newgen.erase(it);	    
      }
  }
};

/** a truncate class based on a repeated deterministic (reverse!) tournament
To be used in SSGA-like replacements (e.g. see eoSSGADetTournamentReplacement)
*/
template <class EOT> 
class eoDetTournamentTruncate : public eoReduce<EOT>
{
public:
  eoDetTournamentTruncate(unsigned _t_size):
    t_size(_t_size)
  {
    if (t_size < 2)
      { 
	cout << "Warning, Size for eoDetTournamentTruncate adjusted to 2\n";
	t_size = 2;
      }
  }

  void operator()(eoPop<EOT>& _newgen, unsigned _newsize)
  {
    unsigned oldSize = _newgen.size();
    if (_newsize == 0)
      {
	_newgen.resize(0);
	return;
      }
    if (oldSize == _newsize)
      return;
    if (oldSize < _newsize)
      throw std::logic_error("eoDetTournamentTruncate: Cannot truncate to a larger size!\n");

    // Now OK to erase some losers
    for (unsigned i=0; i<oldSize - _newsize; i++)
      {
	EOT & eo = inverse_deterministic_tournament<EOT>(_newgen, t_size);
	_newgen.erase(&eo);
      }
  }
private:
  unsigned t_size;
};

/** a truncate class based on a repeated deterministic (reverse!) tournament
To be used in SSGA-like replacements (e.g. see eoSSGAStochTournamentReplacement)
*/
template <class EOT> 
class eoStochTournamentTruncate : public eoReduce<EOT>
{
public:
  eoStochTournamentTruncate(double _t_rate):
    t_rate(_t_rate)
  {
    if (t_rate <= 0.5)
      { 
	cout << "Warning, Rate for eoStochTournamentTruncate adjusted to 0.51\n";
	t_rate = 0.51;
      }
    if (t_rate > 1)
      {
	cout << "Warning, Rate for eoStochTournamentTruncate adjusted to 1\n";
	t_rate = 1;
      }
  }

  void operator()(eoPop<EOT>& _newgen, unsigned _newsize)
  {
    unsigned oldSize = _newgen.size();
    if (_newsize == 0)
      {
	_newgen.resize(0);
	return;
      }
    if (oldSize == _newsize)
      return;
    if (oldSize < _newsize)
      throw std::logic_error("eoStochTournamentTruncate: Cannot truncate to a larger size!\n");
    // Now OK to erase some losers
    for (unsigned i=0; i<oldSize - _newsize; i++)
      {
	EOT & eo = inverse_stochastic_tournament<EOT>(_newgen, t_rate);
	_newgen.erase(&eo);
      }
  }

private:
  double t_rate;
};

//-----------------------------------------------------------------------------

#endif //eoInsertion_h
