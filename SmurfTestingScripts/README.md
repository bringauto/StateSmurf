# Smurf Testing Script
Python script used for automated StateSmurf testing.

# CompareScenarios
This script runs test-scenarios based on a scenario json file and compares each run's State transitions using [SmurfEvaluator](https://github.com/Melky-Phoe/StateSmurf/tree/master/SmurfEvaluator).  
Test is successful when SmurfEvaluator is successful. The script considers the exit code of the application run.

## Sequence diagram

```mermaid
sequenceDiagram
    User->>Script: Execute smurf_compare_scenarios.py
    Script->>validate_json: Validate scenario JSON
    validate_json-->>Script: JSON Valid
    Script->>setup_env: Setup test environment
    setup_env->>Script: Execute setup commands
    loop For each scenario
        Script->>run_scenarios: Run scenario
        run_scenarios->>run_scenarios: Start the tested executable
        run_scenarios->>run_scenarios: Start actions
        run_scenarios->>run_scenarios: Compare scenario output
        run_scenarios->>Script: Tidy up between scenarios
    end
    Script->>cleanup: Cleanup after all scenarios
    cleanup->>Script: Execute cleanup commands
    Script-->>User: Provide test results
```

## Usage
The scenario json file contains commands that will run before, in between each application run, and after all runs are completed.
Application runs contain the name of a test, timeout in seconds, program arguments and commands to be run in parallel.

All paths used in the scenario json are relative to that file.

**Created directories:** 
- etalons: created when -c option is used; files with raw .log files
- output: raw .log output of test-runs
- aggregated_output: aggregated output of test-runs
- evaluator_output: SmurfEvaluator output containing the comparison between an etalon and an output

## Run
```
python3 smurf_compare_scenarios.py --scenario <path> --evaluator <path> [--executable <path> --output <path> --create-etalons --env <path>]
```  
The first run must be run with the --create-etalons flag! Etalons aren't created automatically when not found because they require human approval. The --executable argument is not needed if it is provided in the scenario file (executable provided as a command line argument has priority over the scenario file). The json file provided for the --env argument is used to parametrize the scenario file (strings used for key names will be replaced by their values in the scenario file if they are contained within brackets of a special STATE_SMURF_ENV[] string).
### Arguments:
- **-s | --scenario**: Path to scenario json file containing run scenarios.
- **-e | --executable**: Path to executable of tested application.
- **--evaluator**: Path to SmurfEvaluator executable.
- **-c | --create-etalons**: Switch to create etalons.
- **-o | --output**: Path to directory, where all output directories are created.
- **--env**: Path to environment json file.

#### Exit codes:
- 0 = Success, everything worked
- 1 = Error
- 2 = Some files are different from etalon.
### Scenarios

#### keys:
- `setup` : list of commands, that are run once at the beginning
- `between_runs` : list of commands, that are run in between each test scenario
- `cleanup` : list of commands, that are run once at the end of all tests
- `default_executable` (optional) : path to the tested executable; the --executable argument has priority over this
- `scenarios` : set of testing scenarios containing:
  - `name` : test name; must be unique
  - `timeout_s` (optional) : time in seconds (can be decimal) after which the run is terminated (SIGTERM). 10 seconds after SIGTERM, SIGKILL is sent 
  - `arguments` (optional) : list of program arguments for the tested executable
  - `actions` (optional) : list of commands, that are run in parallel with the tested executable. If any of the commands exit with a non 0 return code, the test fails. Make sure actions are written in a way, where no action is running after the tested executable ends (whis may lead to commands running during clean-up steps or other tests)

note 1: `setup`, `between_runs`, `cleanup` and `action` keys accept nested arrays of strings as values; each string is a command that will be sequentially launched as a python multiprocessing Process; if the next value in line is an array, commands inside will be launched in a parallel thread

note 2: The STATE_SMURF_ENV[] string, mentioned eariler, can also be used in the keys. 

#### Example
Scenario json:
```json
{
  "setup" : [
    "docker-compose --file=STATE_SMURF_ENV[DOCKER_COMPOSE_PATH] up -d", 
    "echo message > msg.txt"
  ],
  "between_runs" : [
    "docker-compose --file=STATE_SMURF_ENV[DOCKER_COMPOSE_PATH] restart"
  ],
  "default_executable": "STATE_SMURF_ENV[PYTHON_PATH]",
  "scenarios" : [
    {
      "name" : "test1",
      "timeout_s" : 70,
      "arguments" : [
        "--foo", "dir/file", "--bar"
      ],
      "actions": [
        "sleep 2",
        "STATE_SMURF_ENV[PYTHON_PATH] test1.py"
      ]
    },
    {
      "name" : "test2",
      "timeout_s" : 65,
      "arguments" : [
        "--foo", "dir/file2"
      ],
      "actions": [
        "sleep 2",
        "STATE_SMURF_ENV[PYTHON_PATH] test2.py"
      ]
    }
  ],
  "cleanup" : [
    "docker-compose --file=STATE_SMURF_ENV[DOCKER_COMPOSE_PATH] down"
  ]
}
```

Env json:
```json
{
    "PYTHON_PATH": "/usr/bin/python3",
    "DOCKER_COMPOSE_PATH": "./etna/docker-compose.yml"
}
```
