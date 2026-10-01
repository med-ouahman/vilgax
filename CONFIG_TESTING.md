# Configuration Validation Before Startup

Run semantic validation on the completed configuration object after parsing and before opening listeners, starting workers, or serving requests. Parsing checks syntax and value formats; this stage checks that the combined configuration is usable and internally consistent.

## Deterministic Configuration Checks

- **Defaults and initialization:** Verify every field has an intentional default. Distinguish an omitted setting from an explicit `off` where inheritance or defaults apply. Resolve global defaults and per-server overrides into effective values before checking them.
- **Worker settings:** `workers` must be either `auto` or a positive count. Resolve `auto` to a supported CPU count and ensure the result is nonzero and within any implementation limit.
- **Connection limits:** Require positive maximum connections and per-worker limits. Check multiplication and allocation calculations for overflow, and confirm the total capacity is compatible with the configured worker count.
- **Listener definitions:** Require at least one listener for every server that must accept traffic. Check valid address families, nonzero valid ports, valid backlog ranges, and duplicate or conflicting address/port pairs. Define explicitly whether a wildcard address conflicts with a specific address on the same port.
- **Server names:** Reject empty names and invalid host-name syntax. Detect duplicate names on the same effective listener set and define which server is the default when a listener has multiple servers.
- **Filesystem paths:** Check that document roots and configured index/error-page paths are nonempty and syntactically valid. Normalize paths, reject unintended traversal, and detect conflicting or duplicate location paths. Filesystem existence and access permissions belong in environment checks below.
- **HTTP limits:** Require positive request-line, header-size, header-count, request-per-connection, and body-size limits. Check conversions and arithmetic for overflow and reject values above implementation or platform limits.
- **Timeouts:** Require positive values within the representable timer range. Validate request-line, header, body, write, keepalive, and FastCGI timeouts independently; zero should only be allowed if its meaning is explicitly defined.
- **Sendfile settings:** Validate `sendfile_min_size` is in range. Resolve server-level `sendfile` against its default and ensure runtime code supports the effective setting for each serving mode.
- **Redirects and error pages:** Validate redirect status codes against supported redirect codes and require a nonempty destination. Require error-page mappings to have valid status codes and nonempty targets.
- **FastCGI settings:** Require a valid backend address and port when FastCGI is configured. Validate connection/read timeouts and reject incomplete or contradictory backend configuration.
- **Cross-setting consistency:** Check that nested locations are within supported depth, settings are allowed in their scopes, and repeated singleton directives have a documented policy (reject duplicates or define which occurrence wins).

## Environment-Dependent Startup Checks

Perform these checks close to startup, report the specific path or resource that failed, and avoid making unit tests depend on the host machine:

- Confirm configured user and group names resolve and that the process can switch to them when requested.
- Check document roots and referenced files exist with the required type and permissions; verify log and PID-file parent directories are usable.
- Check configured resource limits, such as file descriptors, can support the effective worker and connection limits. Fail clearly or report an explicit reduced-capacity mode; do not silently claim the requested capacity.
- Attempt to create and bind listener sockets, reporting address-in-use and permission failures before workers start. Treat socket binding as the authoritative check rather than relying only on a preflight availability probe.
- Check required runtime dependencies are available. A FastCGI backend reachability check should be an optional health check, not a configuration-validity requirement, unless the product explicitly promises fail-fast backend availability.

## Testing the Validator

- Test each check with a valid object and a narrowly invalid variant; cover boundary values such as zero, maximum supported values, and one beyond the maximum.
- Test interactions: duplicate listeners, wildcard/specific listener conflicts, worker and connection capacity calculations, effective global/server overrides, and location collisions.
- Test that validation reports multiple independent errors where practical, with a field or directive path for each error, instead of stopping at an unhelpful generic message.
- Keep deterministic semantic tests separate from filesystem, identity, resource-limit, and socket tests. Use temporary directories and controlled resources for environment-dependent tests.
- Verify invalid configuration is rejected before any listener is bound or worker is launched, and that a failed validation does not partially mutate the effective configuration.