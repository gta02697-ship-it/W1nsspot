// Логика админки
function openAdminLogin() {
    if (localStorage.getItem('isAdmin') === 'true') {
        document.getElementById('adminPanel').style.display = 'flex';
        document.getElementById('adminBubble').style.display = 'block';
        refreshLogs();
        return;
    }
    var pass = prompt("Введите пароль:");
    if (pass === "1512") {
        localStorage.setItem('isAdmin', 'true');
        document.getElementById('adminBubble').style.display = 'block';
        document.getElementById('adminPanel').style.display = 'flex';
        refreshLogs();
    } else if (pass !== null) {
        alert("Неверный пароль!");
    }
}
function closeAdmin() { document.getElementById('adminPanel').style.display = 'none'; }
function clearChat() {
    if (confirm("Очистить чат?")) {
        fetch('/clear_chat', { method: 'POST' }).then(() => {
            alert("Чат очищен!");
            document.querySelectorAll('.messages').forEach(el => el.innerHTML = '');
        });
    }
}
function toggleAdminBubble() {
    var b = document.getElementById('adminBubble');
    b.style.display = (b.style.display === 'block') ? 'none' : 'block';
}
function refreshLogs() {
    var area = document.getElementById('logArea');
    if(!area) return;
    area.value = "Загрузка...";
    fetch('/get_logs').then(r => r.json()).then(d => {
        area.value = d.join('\n');
        area.scrollTop = area.scrollHeight;
    }).catch(e => area.value = "Ошибка: " + e);
}
document.addEventListener('DOMContentLoaded', () => {
    if (localStorage.getItem('isAdmin') === 'true') {
        var b = document.getElementById('adminBubble');
        if(b) b.style.display = 'block';
    }
});
