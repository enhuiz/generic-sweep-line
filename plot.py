import json
import matplotlib.pyplot as plt
from matplotlib import collections
from matplotlib import patches
import sys


def plot_points(ax, points):
    for point in points:
        ax.add_patch(patches.Circle(point, 0.01, color='black'))
    return ax


def plot_polygon(ax, points, color='green'):
    polygon = patches.Polygon(points, alpha=1, ec=color, fc='none', lw=0.2)
    ax.add_patch(polygon)
    return ax

def plot_lines(ax, points, color='green'):
    return plot_polygon(ax, points)

def show(plotter, *args):
    fig, ax = plt.subplots()
    plotter(ax, *args)
    ax.autoscale()
    ax.axis('equal')
    plt.show()


def plot_unit(ax, unit):
    color = unit['color'] or 'green'
    points = unit['points']
    if unit['type'] == 'pt':
        plot_points(ax, unit['points'])
    elif unit['type'] == 'ln':
        plot_polygon(ax, unit['points'])
        plot_points(ax, unit['points'])


def main():
    if(len(sys.argv) > 1):
        content = open(sys.argv[1]).read()
        data = json.loads('{{ "data": {} }}'.format(content))['data']

        fig, ax = plt.subplots()

        plot_points(ax, data['points'])
        for line in data['lines']:
            plot_lines(ax, line)

        ax.autoscale()
        ax.axis('equal')
        plt.show()


if __name__ == '__main__':
    main()
