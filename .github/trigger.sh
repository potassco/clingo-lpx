#!/bin/bash

function list() {
    curl \
      -X GET \
      -H "Accept: application/vnd.github.v3+json" \
      "https://api.github.com/repos/potassco/clingo-lpx/actions/workflows" \
      -d "{\"ref\":\"ref\"}"
}

function dispatch() {
    token=$(grep -A1 workflow_dispatch ~/.tokens | tail -n 1)
    curl \
      -u "rkaminsk:$token" \
      -X POST \
      -H "Accept: application/vnd.github.v3+json" \
      "https://api.github.com/repos/potassco/clingo-lpx/actions/workflows/$1/dispatches" \
      -d "{\"ref\":\"$3\",\"inputs\":{\"wip\":\"$2\"${4:+,$4}}}"
}

branch=master
wip=true

case $1 in
    list)
        list
        ;;
    release)
        if [[ $# < 2 ]]; then
            echo "usage: trigger release REF"
            exit 1
        fi
        wip=false
        branch=$2
        ;&
    dev)
        # .github/workflows/conda-dev.yml
        dispatch 5943107 $wip $branch
        # .github/workflows/ppa-dev.yml
        dispatch 5947898 $wip $branch
        # .github/workflows/manylinux.yml
        dispatch 14968683 $wip $branch
        # .github/workflows/ppawinmac.yml
        dispatch 14968682 $wip $branch
        ;;
    *)
        echo "usage: trigger {list,dev,release}"
        exit 1
        ;;
esac
