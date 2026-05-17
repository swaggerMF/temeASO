using System.Diagnostics;
using Microsoft.AspNetCore.Mvc;
using TodoApp.Models;

namespace TodoApp.Controllers;

public class HomeController : Controller
{
    private const int MaxItemsLimit = 20;
    private readonly ILogger<HomeController> _logger;
    private static readonly List<TodoItem> Todos = new();
    private static int _nextId = 1;
    private static readonly CircularBuffer<string> HistoryLog = new(MaxItemsLimit);

    public HomeController(ILogger<HomeController> logger)
    {
        _logger = logger;
    }

    public IActionResult Index()
    {
        ViewBag.History = HistoryLog.GetAll().Reverse().ToList();
        var model = Todos.OrderBy(t => t.IsDone).ThenBy(t => t.Id).ToList();
        return View(model);
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Add(string title)
    {
        if (!string.IsNullOrWhiteSpace(title))
        {
            var cleanTitle = title.Trim();
            Todos.Add(new TodoItem
            {
                Id = _nextId++,
                Title = cleanTitle,
                IsDone = false
            });
            HistoryLog.Enqueue($"Added task: '{cleanTitle}'");

            if (Todos.Count > MaxItemsLimit)
            {
                var removed = Todos[0];
                Todos.RemoveAt(0);
                HistoryLog.Enqueue($"Auto-removed oldest task: '{removed.Title}'");
            }
        }

        return RedirectToAction(nameof(Index));
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Toggle(int id)
    {
        var todo = Todos.FirstOrDefault(t => t.Id == id);
        if (todo is not null)
        {
            todo.IsDone = !todo.IsDone;
            var status = todo.IsDone ? "completed" : "active";
            HistoryLog.Enqueue($"Marked '{todo.Title}' as {status}");
        }

        return RedirectToAction(nameof(Index));
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Delete(int id)
    {
        var todo = Todos.FirstOrDefault(t => t.Id == id);
        if (todo is not null)
        {
            Todos.Remove(todo);
            HistoryLog.Enqueue($"Deleted task: '{todo.Title}'");
        }

        return RedirectToAction(nameof(Index));
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Edit(int id, string title)
    {
        var todo = Todos.FirstOrDefault(t => t.Id == id);
        if (todo is not null && !string.IsNullOrWhiteSpace(title))
        {
            var newTitle = title.Trim();
            if (!string.Equals(todo.Title, newTitle, StringComparison.Ordinal))
            {
                HistoryLog.Enqueue($"Renamed '{todo.Title}' to '{newTitle}'");
                todo.Title = newTitle;
            }
        }

        return RedirectToAction(nameof(Index));
    }

    public IActionResult Privacy()
    {
        return View();
    }

    [ResponseCache(Duration = 0, Location = ResponseCacheLocation.None, NoStore = true)]
    public IActionResult Error()
    {
        return View(new ErrorViewModel { RequestId = Activity.Current?.Id ?? HttpContext.TraceIdentifier });
    }
}
