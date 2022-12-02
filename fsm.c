/*
 * @file
 * @created 22-11-2022
 * @lastModified  22-11-2022
 * @description This file contains the implementation of FSM
 * @author  Unknown
 */

#include <stdlib.h>
#include "fsm.h"

/*
 * @function-public
 * @description This function creates the fsm_t object
 * @input
 *  (int)state -> Initial state in the FSM
 *  (fsm_trans_t*)tt -> Transition table
 *  (void*)user_data -> User data of any type
 * @output
 *  (fsm_t*)this
 */
fsm_t*
fsm_new (int state, fsm_trans_t* tt, void* user_data)
{
  fsm_t* this = (fsm_t*) malloc (sizeof (fsm_t));
  fsm_init (this, state, tt, user_data);
  return this;
}

/*
 * @function-private
 * @description This function initializes the fsm_t object
 * @input
 *  (fsm_t*)this -> pointer to the fsm_t already allocated
 *  (int)state -> Initial state in the FSM
 *  (fsm_trans_t*)tt -> Transition table
 *  (void*)user_data -> User data of any type
 */
void
fsm_init (fsm_t* this, int state, fsm_trans_t* tt, void* user_data)
{
  this->current_state = state;
  this->tt = tt;
  this->user_data = user_data;
}

/*
 * @function-public
 * @description This function the memory allocated for the FSM
 * @input
 *  (fsm_t*)this -> pointer to the fsm_t to be erased
 */
void
fsm_destroy (fsm_t* this)
{
  free(this);
}

/*
 * @function-public
 * @description This function runs a check on the FSm to see if there is any possible action
 * @input
 *  (fsm_t*)this -> pointer to the fsm_t
 */
void
fsm_fire (fsm_t* this)
{
  fsm_trans_t* t;
  for (t = this->tt; t->orig_state >= 0; ++t) {
    if ((this->current_state == t->orig_state) && t->in(this)) {
      this->current_state = t->dest_state;
      if (t->out)
        t->out(this);
      break;
    }
  }
}

