#include <core.p4>
#include <ebpf_model.p4>

struct Headers_t {
}

parser prs(packet_in p, out Headers_t headers) {
    state start {
        transition accept;
    }
}

control pipe(inout Headers_t headers, out bool pass) {
    bool x_0;
    @name("Reject") action Reject_0(bool rej) {
        pass = rej;
    }
    apply {
        x_0 = true;
        Reject_0(x_0);
    }
}

ebpfFilter<Headers_t>(prs(), pipe()) main;
