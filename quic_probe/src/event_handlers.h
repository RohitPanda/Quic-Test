//
// Created by sergei on 16.05.18.
//

#ifndef QUIC_PROBE_EVENT_HANDLERS_H
#define QUIC_PROBE_EVENT_HANDLERS_H

void schedule_engine(quic_engine_parameters* quic_engine_ref);
void timer_handler (int fd, short what, void *arg);
//void keep_alive_handler (int fd, short what, void *arg);
//void keep_alive(quic_engine_parameters* quic_engine_ref);
#endif //QUIC_PROBE_EVENT_HANDLERS_H
