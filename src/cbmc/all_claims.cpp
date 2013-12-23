/*******************************************************************\

Module: Symbolic Execution of ANSI-C

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <iostream>

#include <util/time_stopping.h>

#include <solvers/sat/satcheck.h>
#include <solvers/prop/cover_goals.h>
#include <solvers/prop/literal_expr.h>

#include <goto-symex/xml_goto_trace.h>

#include "bmc.h"
#include "bv_cbmc.h"

/*******************************************************************\

Function: bmct::all_claims

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

struct goalt
{
  // a property holds if all instances of it are true
  exprt::operandst conjuncts;
  std::string description;

  explicit goalt(const goto_programt::instructiont &instruction)
  {
    description=id2string(instruction.location.get_comment());
  }
  
  goalt()
  {
  }
};

bool bmct::all_claims(
  const goto_functionst &goto_functions,
  prop_convt &solver)
{
  status() << "Passing problem to " << solver.decision_procedure_text() << eom;

  solver.set_message_handler(get_message_handler());
  solver.set_verbosity(get_verbosity());

  // stop the time
  fine_timet sat_start=current_time();
  
  do_conversion(solver);  
  
  // Collect _all_ goals in `goal_map'.
  // This maps claim IDs to 'goalt'
  typedef std::map<irep_idt, goalt> goal_mapt;
  goal_mapt goal_map;
  
  forall_goto_functions(f_it, goto_functions)
    forall_goto_program_instructions(i_it, f_it->second.body)
      if(i_it->is_assert())
        goal_map[i_it->location.get_claim()]=goalt(*i_it);

  // get the conditions for these goals from formula
  
  unsigned claim_counter=0;

  // collect all 'instances' of the properties
  for(symex_target_equationt::SSA_stepst::iterator
      it=equation.SSA_steps.begin();
      it!=equation.SSA_steps.end();
      it++)
  {
    if(it->is_assert())
    {
      irep_idt claim_id;

      if(it->source.pc->is_assert())
        claim_id=it->source.pc->location.get_claim();
      else
      {
        // need new claim ID, say for an unwinding assertion
        claim_counter++;
        claim_id=i2string(claim_counter);
        goal_map[claim_id].description=it->comment;
      }
      
      goal_map[claim_id].conjuncts.push_back(
        literal_exprt(it->cond_literal));
    }
  }
  
  cover_goalst cover_goals(solver);
  
  for(goal_mapt::const_iterator
      it=goal_map.begin();
      it!=goal_map.end();
      it++)
  {
    // Our goal is to falsify a property.
    // The following is TRUE if the conjunction is empty.
    literalt p=!solver.convert(conjunction(it->second.conjuncts));
    cover_goals.add(p);
  }

  status() << "Running " << solver.decision_procedure_text() << eom;

  cover_goals();  

  // output runtime

  {
    fine_timet sat_stop=current_time();
    status() << "Runtime decision procedure: "
             << (sat_stop-sat_start) << "s" << eom;
  }
  
  // report
  if(ui!=ui_message_handlert::XML_UI)
  {
    status() << eom;
    status() << "** Results:" << eom;
  }
  
  std::list<cover_goalst::cover_goalt>::const_iterator g_it=
    cover_goals.goals.begin();
    
  for(goal_mapt::const_iterator
      it=goal_map.begin();
      it!=goal_map.end();
      it++, g_it++)
  {
    if(ui==ui_message_handlert::XML_UI)
    {
      xmlt xml_result("result");
      xml_result.set_attribute("claim", id2string(it->first));

      xml_result.set_attribute("status",
        g_it->covered?"FAILURE":"SUCCESS");
    
      std::cout << xml_result << "\n";
    }
    else
    {
      status() << "[" << it->first << "] "
               << it->second.description << ": " << (g_it->covered?"FAILED":"OK")
               << eom;
    }
  }

  status() << eom;
  
  status() << "** " << cover_goals.number_covered()
           << " of " << cover_goals.size() << " failed ("
           << cover_goals.iterations() << " iterations)" << eom;
  
  return false;
}

