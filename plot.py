import sys
import json
import itertools
import matplotlib.pyplot as plt
from matplotlib import collections
from matplotlib import patches


def plot_points(ax, points):
    for point in points:
        xy = point['xy']
        radius = 0.02 if 'radius' not in point else point['radius']
        color = 'black' if 'color' not in point else point['color']
        ax.add_patch(patches.Circle(point['xy'], radius=radius, color=color))
    return ax


def plot_linked_points(ax, points, color='green'):
    points = list(points)
    for a, b in zip(points[:-1], points[1:]):
        color = 'black' if 'color' not in a else a['color']
        ax.add_patch(patches.ConnectionPatch(
            a['xy'], b['xy'], 'data', lw=1, color='green'))
    return ax


def show(plotter, *args):
    fig, ax = plt.subplots()
    plotter(ax, *args)
    ax.autoscale()
    ax.axis('equal')
    plt.show()


def main():
    if(len(sys.argv) > 1):
        points = json.load(open(sys.argv[1]))['points']

        fig, ax = plt.subplots()

        for lnk_id, points in itertools.groupby(points, lambda p: p['lnk_id']):
            if lnk_id < 0:
                plot_points(ax, points)
            else:
                plot_linked_points(ax, points)

        ax.autoscale()
        ax.axis('equal')
        plt.show()


if __name__ == '__main__':
    main()
