/*
 * quic_engine_holder.h
 *
 *  Created on: Apr 13, 2018
 *      Author: sergei
 */


#ifndef SRC_QUIC_ENGINE_HOLDER_H_
#define SRC_QUIC_ENGINE_HOLDER_H_



#include <lsquic.h>

#include "engine_structs.h"

int init_engine(quic_engine_parameters* quic_engine_ref);
void destroy_engine(quic_engine_parameters* quic_engine_ref);

#endif /* SRC_QUIC_ENGINE_HOLDER_H_ */
