#!/bin/bash

PORT=12345

show_ss() {
    clear
    echo -n "Socket Statistics for Port: "

    ss_output=$(ss -tan | grep ":$PORT")

    colored_output=$(echo "$ss_output" | awk -v port="$PORT" '
        BEGIN {
            time_wait_color="\033[92m"  # Set color to green for TIME-WAIT
            established_color="\033[93m"  # Set color to yellow for ESTABLISHED
            listen_color="\033[91m"  # Set color to red for LISTENING
            default_color="\033[97m"  # Set color to white for other states
        }
        /TIME-WAIT/ {
            gsub(port, time_wait_color port "\033[0m");
            print;
        }
        /ESTAB/ {
            gsub(port, established_color port "\033[0m");
            print;
        }
        /LISTEN/ {
            gsub(port, listen_color port "\033[0m");
            print;
        }
        !/TIME-WAIT|ESTAB|LISTEN/ {
            gsub(port, default_color port "\033[0m");
            print;
        }
    ')

    echo -e "\033[97m$PORT\e[0m"
    echo "$colored_output"
}

while true; do
    show_ss
    echo "Press 'q' to quit..."
    read -t 1 -n 1 input
    if [[ $input = "q" ]]; then
        break
    fi
done

clear
