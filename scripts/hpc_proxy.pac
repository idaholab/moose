// INL HPC PAC (Proxy-Auto Configuration)
//
// Used within the MOOSE HPC connectivity instructions
// (mooseframework.org/help/inl/hpc_remote.html)
// to tunnel traffic to common HPC resources
// using a SOCKS proxy to hpclogin.inl.gov.
// See the link for more information.

function FindProxyForURL(url, host) {
  // For *.hpc.inl.gov, first attempt to use the SOCKS proxy.
  // If no success, attempt a direct connection (useful when on the INL VPN)
  if (dnsDomainIs(host, ".hpc.inl.gov"))
    return "SOCKS5 localhost:5555; DIRECT";
  // For all other domains, use direct access (no proxy)
  return "DIRECT";
}
