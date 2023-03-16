# ThreadedGeneralUserObject

ThreadedGeneralUserObjects are "UserObjects" (custom objects or algorithms) that are not associated with any mesh
entity, but where n-thread copies are created for the purpose of running threaded calculations or avoiding
thread-locks for lookup operations.