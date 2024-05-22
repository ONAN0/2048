#!/bin/bash

get_state_word() {
    case $1 in
        R) echo "Running";;
        S) echo "Sleeping";;
        D) echo "Disk Sleep";;
        Z|"<defunct>") echo "Zombie";;
        T) echo "Stopped";;
        W) echo "Pageant";;
        X) echo "Dead";;
        *) echo "$1";;
    esac
}

filename="PID.txt"

# Kontrola existencie súboru
if [ ! -f "$filename" ]; then
    echo "File not found: $filename"
    exit 1
fi

while true; do
    clear

    echo "   PID | USER |         COMMAND         |      STATUS"

    # Cita PID zo suboru a vypisuje informacie
    while read -r pid; do
        if [ -n "$pid" ]; then
            process_info=$(ps -p $pid -o pid,user,cmd,state --no-headers)
            if [ -n "$process_info" ]; then
                state=$(echo "$process_info" | awk '{print $4}')
                state_word=$(get_state_word "$state")
                echo "-----------------------------------------------------------"
                echo "$process_info $state_word"
            else
                echo "Process with PID $pid not found."
            fi
        fi
    done < "$filename"

    echo "Press 'q' to quit..."
    
    read -t 1 -n 1 key

    if [ "$key" == "q" ]; then
        break
    fi
done

clear
