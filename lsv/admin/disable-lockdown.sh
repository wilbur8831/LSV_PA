if [ -f ".github/workflows/lockdown.yml" ]; then
    git sw master
    mv .github/workflows/lockdown.yml .github/workflows/lockdown.yml.disabled
    git a .github/workflows
    git cm "Disable lockdown"
    git push
    lsv/admin/update-branches-master.sh
else
    echo "[WARNING] Lockdown is NOT enabled!"
fi
