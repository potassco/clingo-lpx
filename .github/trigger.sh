#!/bin/bash

dev_branch=master

function list() {
    curl \
      -X GET \
      -H "Accept: application/vnd.github.v3+json" \
      "https://api.github.com/repos/potassco/clingo-lpx/actions/workflows" \
      -d "{\"ref\":\"${dev_branch}\"}"
}

function dispatch() {
    token=$(grep -A1 workflow_dispatch ~/.tokens | tail -n 1)
    curl \
      -u "rkaminsk:$token" \
      -X POST \
      -H "Accept: application/vnd.github.v3+json" \
      "https://api.github.com/repos/potassco/clingo-lpx/actions/workflows/$1/dispatches" \
      -d "{\"ref\":\"${dev_branch}\"}"
}

case $1 in
    list)
        list
        ;;
    dev)
        # .github/workflows/conda-dev.yml
        dispatch 5943107
        # .github/workflows/ppa-dev.yml
        dispatch 5947898
        ;;
    release)
        echo "implement me"
        ;;
    *)
        echo "usage: trigger {list,dev,release}"
        ;;
esac
