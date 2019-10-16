_pvesc_completion()
{
	local cur prev
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	case ${COMP_CWORD} in
		1)
			COMPREPLY=($(compgen -W "help container deploy version build" -- ${cur}))
			;;
		2)
			case ${prev} in
				help)
					COMPREPLY=($(compgen -W "" -- ${cur}))
					;;
				container)
					COMPREPLY=($(compgen -W "help build check" -- ${cur}))
					;;
				deploy)
					COMPREPLY=($(compgen -W "deploy global-config help" -- ${cur}))
					;;
				version)
					COMPREPLY=($(compgen -W "" -- ${cur}))
					;;
			esac
			;;
		*)
			COMPREPLY=()
			;;
	esac
}

complete -F _pvesc_completion pvesc